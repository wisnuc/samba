/* 
 * notifying VFS module for samba.  Log selected file operations to syslog
 * facility.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#include "includes.h"
#include "system/filesys.h"
#include "system/syslog.h"
#include "smbd/smbd.h"
#include "lib/param/loadparm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <attr/xattr.h>

#undef DBGC_CLASS
#define DBGC_CLASS DBGC_VFS


#define CACHELEN 1024
#define USERNAMELEN 128
#define UUIDLEN 64
#define SERVERPORT 6969 //服务器监听端口号
#define IPADDRESS "127.0.0.1"

char buf[CACHELEN] = {};
char userName[USERNAMELEN] = {};

int netflags = 1; //deal with error for network, if samba net can not work ,netflags will be zero

static int sendmessage(const char* message){

	char buf[16] = {};
	int sockfd;
	struct sockaddr_in client_addr;
	
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("sock error");
		return -1;
	}

	//set zero
	memset(&client_addr,0,sizeof(struct sockaddr_in));

	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(SERVERPORT);
	client_addr.sin_addr.s_addr = inet_addr(IPADDRESS);
	
	//connect to server
	if(connect(sockfd,(struct sockaddr*)&client_addr,sizeof(struct sockaddr)) < 0){
		perror("connect error");
		close(sockfd);
		return -1;
	}

	if(send(sockfd,message,strlen(message),0) < 0){
		perror("send error!");
		close(sockfd);
		return -1;
	}
	
	recv(sockfd,buf,sizeof(buf),0);
	// printf("buf:%s\n",buf);
	if(strncmp(buf,"OK!",3)){
		// printf("the order is failed!please try again!\n");
		close(sockfd);
		return -1;
	}
	close(sockfd);
	return 0;
}

static int get_uuid(char* uuid, int len){
	FILE* fp = popen("uuidgen","r");
	if(fp == NULL){
		return -1;
	}
	fgets(uuid,len,fp);
	*(uuid+strlen(uuid)-1) = '\0';
	pclose(fp);
	return 0;
}

static int get_uuid_file(char* path, char* name, int len){
	int result = getxattr(path,"user.uuid",name,len);
	return 0;
}

static int my_notify_syslog_facility(vfs_handle_struct *handle)
{
	static const struct enum_list enum_log_facilities[] = {
		{ LOG_USER, "USER" },
		{ LOG_LOCAL0, "LOCAL0" },
		{ LOG_LOCAL1, "LOCAL1" },
		{ LOG_LOCAL2, "LOCAL2" },
		{ LOG_LOCAL3, "LOCAL3" },
		{ LOG_LOCAL4, "LOCAL4" },
		{ LOG_LOCAL5, "LOCAL5" },
		{ LOG_LOCAL6, "LOCAL6" },
		{ LOG_LOCAL7, "LOCAL7" },
		{ -1, NULL}
	};

	int facility;

	facility = lp_parm_enum(SNUM(handle->conn), "my_notify", "facility", enum_log_facilities, LOG_USER);

	return facility;
}


static int my_notify_syslog_priority(vfs_handle_struct *handle)
{
	static const struct enum_list enum_log_priorities[] = {
		{ LOG_EMERG, "EMERG" },
		{ LOG_ALERT, "ALERT" },
		{ LOG_CRIT, "CRIT" },
		{ LOG_ERR, "ERR" },
		{ LOG_WARNING, "WARNING" },
		{ LOG_NOTICE, "NOTICE" },
		{ LOG_INFO, "INFO" },
		{ LOG_DEBUG, "DEBUG" },
		{ -1, NULL}
	};

	int priority;

	priority = lp_parm_enum(SNUM(handle->conn), "my_notify", "priority",
				enum_log_priorities, LOG_NOTICE);
	if (priority == -1) {
		priority = LOG_WARNING;
	}

	return priority;
}

/* Implementation of vfs_ops.  Pass everything on to the default
   operation but log event first. */

static int my_notify_connect(vfs_handle_struct *handle, const char *svc, const char *user)
{
	if(netflags){
		int result;
	
		snprintf(userName,USERNAMELEN,"%s",user);  // get user name
	
		result = SMB_VFS_NEXT_CONNECT(handle, svc, user);
		if (result < 0) {
			return result;
		}
		
		openlog("smbd_my_notify", LOG_PID, my_notify_syslog_facility(handle));
	
		syslog(my_notify_syslog_priority(handle), "user %s connect to service %s by user %s\n", 
		       userName, svc, user);
	
		return 0;
	}
	return -1;
}

static void my_notify_disconnect(vfs_handle_struct *handle)
{
	syslog(my_notify_syslog_priority(handle), "user %s disconnected\n", userName);
	SMB_VFS_NEXT_DISCONNECT(handle);

	return;
}

static DIR *my_notify_opendir(vfs_handle_struct *handle, const char *fname, const char *mask, uint32_t attr)
{
	DIR *result;
	
	result = SMB_VFS_NEXT_OPENDIR(handle, fname, mask, attr);

	syslog(my_notify_syslog_priority(handle), "user %s opendir %s %s%s\n",
	       userName, fname,
	       (result == NULL) ? "failed: " : "",
	       (result == NULL) ? strerror(errno) : "");

	return result;
}

static int my_notify_mkdir(vfs_handle_struct *handle, const char *path, mode_t mode)
{
	if(netflags){
		int result, res;
		char uuidpath[128];
		char uuidbuf[UUIDLEN];

		result = SMB_VFS_NEXT_MKDIR(handle, path, mode);

		if(!result){
			get_uuid(uuidbuf,UUIDLEN);	
			snprintf(uuidpath,sizeof(uuidpath),"%s/%s", handle->conn->cwd,path);
			snprintf(buf,sizeof(buf),"user\t%s\toperation\tmkdir\tsharepath\t%s\tpath\t%s\tuuid\t%s",userName,handle->conn->cwd,path,uuidbuf);
			setxattr(uuidpath, "user.uuid", uuidbuf, strlen(uuidbuf),0);
			res = sendmessage(buf);
			if(res == -1){
				netflags = 0;
			}
		}

		syslog(my_notify_syslog_priority(handle), "user %s mkdir %s %s%s\n", 
	      userName, path,
	      (result < 0) ? "failed: " : "",
	      (result < 0) ? strerror(errno) : "");

		return result;
	}
	
	return -1;
}

static int my_notify_rmdir(vfs_handle_struct *handle, const char *path)
{
	if(netflags){
		int result, res;
		char uuidpath[128] = {};
		char uuidbuf[UUIDLEN] = {};

		snprintf(uuidpath,sizeof(uuidpath),"%s/%s", handle->conn->cwd,path);

		getxattr(uuidpath,"user.uuid",uuidbuf,UUIDLEN);
		result = SMB_VFS_NEXT_RMDIR(handle, path);
		snprintf(buf,sizeof(buf),"user\t%s\toperation\trmdir\tsharepath\t%s\tpath\t%s\tuuid\t%s",userName,handle->conn->cwd,path,uuidbuf);
		if(!result){
			res = sendmessage(buf);
			if(res == -1){
				netflags = 0;
			}
		}

		syslog(my_notify_syslog_priority(handle), "user %s rmdir %s %s%s\n", 
		       userName, path, 
		       (result < 0) ? "failed: " : "",
		       (result < 0) ? strerror(errno) : "");
	
		return result;
	}
	return -1;
}

static int my_notify_open(vfs_handle_struct *handle,
		      struct smb_filename *smb_fname, files_struct *fsp,
		      int flags, mode_t mode)
{
	if(netflags){
		int result, res;
		
		result = SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);

		if(result >= 0){
			if(flags == 194){
				char uuidpath[128];
				char uuidbuf[UUIDLEN];
				get_uuid(uuidbuf,UUIDLEN);	
				snprintf(uuidpath,sizeof(uuidpath),"%s/%s", handle->conn->cwd,smb_fname->base_name);
				setxattr(uuidpath, "user.uuid", uuidbuf, strlen(uuidbuf),0);
				snprintf(buf,sizeof(buf),"user\t%s\toperation\tcreatefile\tsharepath\t%s\tpath\t%s\tuuid\t%s",userName,handle->conn->cwd,smb_fname->base_name,uuidbuf);
				res = sendmessage(buf);
				if(res){
					netflags = 0;
				}
			}
		}
	
		syslog(my_notify_syslog_priority(handle), "user %s flages %d open %s (fd %d) %s%s%s\n", 
		       userName, flags, smb_fname->base_name, result,
		       ((flags & O_WRONLY) || (flags & O_RDWR)) ? "for writing " : "", 
		       (result < 0) ? "failed: " : "",
		       (result < 0) ? strerror(errno) : "");
	
		return result;
	}
	return -1;
}

static int my_notify_close(vfs_handle_struct *handle, files_struct *fsp)
{
	int result;

	result = SMB_VFS_NEXT_CLOSE(handle, fsp);

	syslog(my_notify_syslog_priority(handle), "user %s close fd %d %s%s\n",
	       userName, fsp->fh->fd,
	       (result < 0) ? "failed: " : "",
	       (result < 0) ? strerror(errno) : "");

	return result;
}

static int my_notify_rename(vfs_handle_struct *handle,
			const struct smb_filename *smb_fname_src,
			const struct smb_filename *smb_fname_dst)
{
	if(netflags){
		int result, res;
		char uuidpath[128] = {};
		char uuidbuf[UUIDLEN] = {};

		snprintf(uuidpath,sizeof(uuidpath),"%s/%s", handle->conn->cwd,smb_fname_src->base_name);
		getxattr(uuidpath,"user.uuid",uuidbuf,UUIDLEN);
	
		snprintf(buf,sizeof(buf),"user\t%s\toperation\trename\tsharepath\t%s\tsrc\t%s\tdst\t%s\tuuid\t%s",userName,handle->conn->cwd,smb_fname_src->base_name,smb_fname_dst->base_name,uuidbuf);
	
		result = SMB_VFS_NEXT_RENAME(handle, smb_fname_src, smb_fname_dst);
		if(!result){
			res = sendmessage(buf);
			if(res == -1){
				netflags = 0;
			}
		}
	
		syslog(my_notify_syslog_priority(handle), "user %s rename %s -> %s %s%s\n",
		       userName, smb_fname_src->base_name,
		       smb_fname_dst->base_name,
		       (result < 0) ? "failed: " : "",
		       (result < 0) ? strerror(errno) : "");
	
		return result;
	}
	return -1;  
}

static int my_notify_unlink(vfs_handle_struct *handle,
			const struct smb_filename *smb_fname)
{
	if(netflags){
		int result, res;
		char uuidpath[128] = {};
		char uuidbuf[UUIDLEN] = {};

		snprintf(uuidpath,sizeof(uuidpath),"%s/%s", handle->conn->cwd,smb_fname->base_name);
		getxattr(uuidpath,"user.uuid",uuidbuf,UUIDLEN);

		result = SMB_VFS_NEXT_UNLINK(handle, smb_fname);
		snprintf(buf,sizeof(buf),"user\t%s\toperation\tdeletefile\tsharepath\t%s\tpath\t%s\tuuid\t%s",userName, handle->conn->cwd, smb_fname->base_name, uuidbuf);

		if(!result){
			res = sendmessage(buf);
			if(res == -1){
				netflags = 0;
			}
		}
	
		syslog(my_notify_syslog_priority(handle), "user %s unlink %s %s%s\n",
		       userName,smb_fname->base_name,
		       (result < 0) ? "failed: " : "",
		       (result < 0) ? strerror(errno) : "");
	
		return result;
	}
	return -1;
}

static int my_notify_chmod(vfs_handle_struct *handle, const char *path, mode_t mode)
{
	// int result;

	// result = SMB_VFS_NEXT_CHMOD(handle, path, mode);

	// syslog(my_notify_syslog_priority(handle), "chmod %s mode 0x%x %s%s\n",
	//        path, mode,
	//        (result < 0) ? "failed: " : "",
	//        (result < 0) ? strerror(errno) : "");

	// return result;
	return -1;
}

static int my_notify_chmod_acl(vfs_handle_struct *handle, const char *path, mode_t mode)
{
	// int result;

	// result = SMB_VFS_NEXT_CHMOD_ACL(handle, path, mode);

	// syslog(my_notify_syslog_priority(handle), "chmod_acl %s mode 0x%x %s%s\n",
	//        path, mode,
	//        (result < 0) ? "failed: " : "",
	//        (result < 0) ? strerror(errno) : "");

	// return result;
	return -1;
}

static int my_notify_fchmod(vfs_handle_struct *handle, files_struct *fsp, mode_t mode)
{
	// int result;

	// result = SMB_VFS_NEXT_FCHMOD(handle, fsp, mode);

	// syslog(my_notify_syslog_priority(handle), "fchmod %s mode 0x%x %s%s\n",
	//        fsp->fsp_name->base_name, mode,
	//        (result < 0) ? "failed: " : "",
	//        (result < 0) ? strerror(errno) : "");

	// return result;
	return -1;
}

static int my_notify_fchmod_acl(vfs_handle_struct *handle, files_struct *fsp, mode_t mode)
{
	// int result;

	// result = SMB_VFS_NEXT_FCHMOD_ACL(handle, fsp, mode);

	// syslog(my_notify_syslog_priority(handle), "fchmod_acl %s mode 0x%x %s%s\n",
	//        fsp->fsp_name->base_name, mode,
	//        (result < 0) ? "failed: " : "",
	//        (result < 0) ? strerror(errno) : "");

	// return result;
	return -1;
}

static struct vfs_fn_pointers vfs_my_notify_fns = {
	.connect_fn = my_notify_connect,
	.disconnect_fn = my_notify_disconnect,
	.opendir_fn = my_notify_opendir,
	.mkdir_fn = my_notify_mkdir,
	.rmdir_fn = my_notify_rmdir,
	.open_fn = my_notify_open,
	.close_fn = my_notify_close,
	.rename_fn = my_notify_rename,
	.unlink_fn = my_notify_unlink,
	.chmod_fn = my_notify_chmod,
	.fchmod_fn = my_notify_fchmod,
	.chmod_acl_fn = my_notify_chmod_acl,
	.fchmod_acl_fn = my_notify_fchmod_acl
};

static_decl_vfs;
NTSTATUS vfs_my_notify_init(void)
{
	return smb_register_vfs(SMB_VFS_INTERFACE_VERSION, "my_notify",
				&vfs_my_notify_fns);
}

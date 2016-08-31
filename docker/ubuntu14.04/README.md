# Docker for Samba

### Prerequisite
  1. [**Docker Usage Reference**](https://github.com/JiangWeiGitHub/Docker)
  2. Using ubuntu:latest as base container

### Notice
  1. Expose related ports<p>
  [*Reference*](https://www.samba.org/~tpot/articles/firewall.html)<p>
  
  ```
    udp 137
    udp 138
    tcp 139
    tcp 445
  ```
  2. Command Format<p>
  [*Reference*](https://github.com/docker/docker/issues/7459)<p>

  ```
  docker daemon -H tcp://0.0.0.0:5678
  
  1) SystemD Mode: (Run Ubuntu Only, you have to 'exec' into this containter to install samba and run)
    docker -H tcp://0.0.0.0:5678 run --privileged -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti -e "container=docker" -v /sys/fs/cgroup:/sys/fs/cgroup ubuntu /sbin/init
    
  2) SystemV Mode: (Run Ubuntu Only, you have to 'exec' into this containter to install samba and run)
    docker -H tcp://0.0.0.0:5678 run --privileged -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti ubuntu
    
  3) SystemV Mode: (Run Our Samba Directly, import userList)
    docker -H tcp://0.0.0.0:5678 run -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti -v /home/tmp/:/srv/samba/shareFolder -v /srv/samba/import.file:/home/import.file IMAGEID -n -i "/home/import.file"
    
  4) SystemV Mode: (Run Our Samba Directly, add new user)
    docker -H tcp://0.0.0.0:5678 run -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti -v /home/tmp/:/srv/samba/shareFolder  IMAGEID -n -u "aaaaa:123456" -u "bbbbb:654321"
    ...
    docker -H tcp://0.0.0.0:5678 ps -a
    ...
    docker -H tcp://0.0.0.0:5678 exec -it IMAGEID bash
  ```
  
  ```
  ps:
    1. -n : run smbd daemon or it will only run once and exit
    2. -i : import userList file (just like "/???/???"), HAVE TO use -v to point out the real file where is, just like '-v /srv/samba/import.file:/home/import.file -i "/home/import.file"'
    3. -u : add a new user (just like "aaaaa:123456"), HAVE TO use -v to point out the real file where is, and container path is '/srv/samba/backup/'
    4. Can use all parameters, docker -H tcp://0.0.0.0:5678 run -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti -v /home/tmp/:/srv/samba/backup/ -v /srv/samba/import.file:/home/import.file IMAGEID -i "/home/import.file" -n -u "abc:123456" -u "efg:654321" -u "hij:987654"
  ```
  
  3. Backup User List & All Configurations (Not used in this project)

  ```
    /etc/passwd (User name and account info)
    /etc/shadow (Passwords)
    /etc/group (Group names and membership)
    /etc/gshadow - Contains group encrypted passwords.
    /etc/samba/*
    /var/lib/samba/*
  ```
  
  ```
    Tar (Docker Container One):
    tar -zcf /home/backup.tar.gz /etc/passwd /etc/shadow /etc/group /etc/gshadow /etc/samba/* /var/lib/samba/*
    
    Untar (Docker Container Two):
    File Path: /
    cd /
    tar -zxf /backup.tar.gz
  ```

  4. Only Backup User List with **SMBPASSWD** file
  
  file format:<p>
  ```
    ccccc:1002:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:31D6CFE0D16AE931B73C59D7E0C089C0:[U          ]:LCT-57A84C1E:
    eeeee:1004:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:69943C5E63B4D2C104DBBCC15138B72B:[U          ]:LCT-57A84D0B:
    ddddd:1003:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:3DBDE697D71690A769204BEB12283678:[U          ]:LCT-57A84C25:
    bbbbb:1001:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:4057B60B514C5402DDE3D29A1845C366:[U          ]:LCT-57A84611:
    aaaaa:1000:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:32ED87BDB5FDC5E9CBA88547376818D4:[U          ]:LCT-57A84611:

  ```

  method:<p>
  ```
    export:
      pdbedit -Lw -u USERNAME > /???/filename (file type is not important)
      
    import:
      a. Analyse smbpasswd file, pick up all username filed, use "useradd" command to add every user into system
      b. pdbedit -i smbpasswd:/???/filename
  ```

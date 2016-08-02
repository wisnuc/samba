# Samba

### Host Platform
  1. Ubuntu 16.04.1 server 64bit

### Installation
  1. `apt-get install samba`<p>
  Note: smbd & nmbd version - 4.3.9-Ubuntu

### Operating system requirements
  1. Kernel support
  `cat /boot/config-4.4.0-31-generic`<p>

  ```
    CONFIG_EXT4_FS_XATTR=y
    CONFIG_EXT4_FS_SECURITY=y
    CONFIG_EXT4_FS_POSIX_ACL=y
  ```
  Note: "*CONFIG_EXT4_FS_XATTR*" is not found under Ubuntu 16.04.1.

### File system support
  1. modify /etc/fstab file
  ```
    sample:
    
    /dev/...          /srv/samba/demo          ext4          defaults,barrier=1          1 1
  ```
  Note: The "**barrier=1**" option ensures that tdb transactions are safe against unexpected power loss.
  
### Testing Filesystem
  ```
    Test for xattr:
  
    # touch test.txt
    # setfattr -n user.test -v test test.txt
    # setfattr -n security.test -v test2 test.txt
    
    # getfattr -d test.txt
    # file: test.txt
    user.test="test"
    
    # getfattr -n security.test -d test.txt
    # file: test.txt
    security.test="test2"
  ```

  ```
    Test for ACL:
  
    # touch test.txt
    # setfacl -m g:adm:rwx test.txt
    
    # getfacl test.txt
    # file: test.txt
    # owner: root
    # group: root
    user::rw-
    group::r--
    group:adm:rwx
    mask::rwx
    other::r--
  ```

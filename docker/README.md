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
  SystemD Mode: (Run Ubuntu Only, you have to 'exec' into this containter to install samba and run)
    docker -H tcp://0.0.0.0:5678 run --privileged -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti -e "container=docker" -v /sys/fs/cgroup:/sys/fs/cgroup ubuntu /sbin/init
    
  SystemV Mode: (Run Ubuntu Only, you have to 'exec' into this containter to install samba and run)
    docker -H tcp://0.0.0.0:5678 run --privileged -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti ubuntu
    
  SystemV Mode: (Run Samba Directly)
    docker -H tcp://0.0.0.0:5678 run --privileged -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti -v /home/backup/:/srv/samba/ -v /srv/samba/import.file:/home/import.file IMAGEID -n -i "/home/import.file"
    ...
    docker -H tcp://0.0.0.0:5678 ps -a
    ...
    docker -H tcp://0.0.0.0:5678 exec -it d418c2258ec4 bash
  ```
  
  ps:
    1. -n : run smbd daemon
    2. -i : import userList file (just like "/???/???"), **have to use -v to point out the real file where is**
    3. -u : add a new user (just like "aaaaa:123456")
  
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

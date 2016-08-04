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
    docker -H tcp://0.0.0.0:5678 run --privileged -p 137:137/udp -p 138:138/udp -p 139:139 -p 445:445 -ti -e "container=docker" -v /sys/fs/cgroup:/sys/fs/cgroup ubuntu /sbin/init
    ...
    docker -H tcp://0.0.0.0:5678 ps -a
    ...
    docker -H tcp://0.0.0.0:5678 exec -it d418c2258ec4 bash
  ```
  3. Backup User List & All Configurations

  ```
    /etc/passwd (User name and account info)
    /etc/shadow (Passwords)
    /etc/group (Group names and membership)
    /etc/gshadow - Contains group encrypted passwords.
    /etc/samba/*
    /var/lib/samba/*
  ```

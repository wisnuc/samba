FROM ubuntu:16.04

MAINTAINER JiangWeiGitHub <wei.jiang@winsuntech.cn>

# Update apt sourcelist
RUN echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ xenial main restricted universe multiverse" > /etc/apt/sources.list \
 && echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ xenial-backports main restricted universe multiverse" >> /etc/apt/sources.list \
 && echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ xenial-proposed main restricted universe multiverse" >> /etc/apt/sources.list \
 && echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ xenial-security main restricted universe multiverse" >> /etc/apt/sources.list \
 && echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ xenial-updates main restricted universe multiverse" >> /etc/apt/sources.list

# update apt
RUN apt-get update

# install samba with apt-get
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install \
    samba && \
    echo "[global]" > /etc/samba/smb.conf && \
    echo "        workgroup = WORKGROUP" >> /etc/samba/smb.conf && \
    echo "        netbios name = SAMBA" >> /etc/samba/smb.conf && \
    echo "" >> /etc/samba/smb.conf && \
    echo "        map to guest = Bad User" >> /etc/samba/smb.conf && \
    echo "" >> /etc/samba/smb.conf && \
    echo "        log file = /var/log/samba/%m" >> /etc/samba/smb.conf && \
    echo "        log level = 1" >> /etc/samba/smb.conf && \
    echo "" >> /etc/samba/smb.conf && \
    echo "[guest]" >> /etc/samba/smb.conf && \
    echo "        # This share allows anonymous (guest) access" >> /etc/samba/smb.conf && \
    echo "        # without authentication!" >> /etc/samba/smb.conf && \
    echo "        path = /tmp/samba/guest/" >> /etc/samba/smb.conf && \
    mkdir -p /tmp/samba/guest/ && \
    echo "Welcome to use Samba!" >> /tmp/samba/guest/welcome.txt && \
    echo "        read only = no" >> /etc/samba/smb.conf && \
    echo "        guest ok = yes" >> /etc/samba/smb.conf && \
    echo "" >> /etc/samba/smb.conf && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/*
COPY samba.sh /usr/bin/

VOLUME ["/etc/samba","/backup/samba"]

EXPOSE 137 138 139 445

ENTRYPOINT ["samba.sh"]

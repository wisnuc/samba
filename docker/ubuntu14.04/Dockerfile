FROM ubuntu:14.04.4

MAINTAINER JiangWeiGitHub <wei.jiang@winsuntech.cn>

# Update apt sourcelist
RUN echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ trusty main restricted universe multiverse" > /etc/apt/sources.list \
 && echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ trusty-backports main restricted universe multiverse" >> /etc/apt/sources.list \
 && echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ trusty-proposed main restricted universe multiverse" >> /etc/apt/sources.list \
 && echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ trusty-security main restricted universe multiverse" >> /etc/apt/sources.list \
 && echo "deb http://ubuntu.uestc.edu.cn/ubuntu/ trusty-updates main restricted universe multiverse" >> /etc/apt/sources.list

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
    echo "        path = /srv/samba/guest/" >> /etc/samba/smb.conf && \
    mkdir -p /srv/samba/guest/ && \
    echo "Welcome to use Samba!" >> /srv/samba/guest/welcome.txt && \
    echo "        read only = no" >> /etc/samba/smb.conf && \
    echo "        guest ok = yes" >> /etc/samba/smb.conf && \
    echo "" >> /etc/samba/smb.conf && \
    
    echo "[user]" >> /etc/samba/smb.conf && \
    echo "        # This share requires authentication to access" >> /etc/samba/smb.conf && \
    echo "        path = /srv/samba/user/" >> /etc/samba/smb.conf && \
    mkdir -p /srv/samba/user/ && \
    echo "Hello, Master!" >> /srv/samba/user/welcome.txt && \    
    echo "        read only = no" >> /etc/samba/smb.conf && \
    echo "        guest ok = no" >> /etc/samba/smb.conf && \
    echo "" >> /etc/samba/smb.conf && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/*

# Caution samba.sh permission, chmod 777
COPY samba.sh /usr/bin/

RUN chmod 755 /usr/bin/samba.sh

VOLUME ["/etc/samba","/backup/samba"]

EXPOSE 137 138 139 445

ENTRYPOINT ["samba.sh"]

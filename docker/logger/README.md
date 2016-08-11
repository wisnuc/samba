# Samba Logger with VFS using Audit

### Configuration
  + Samba related 
    - Edit /etc/samba/smb.conf
  
    ```
      [global]
              ...
      ...
      [user]
              ...
              vfs objects = full_audit
              full_audit:prefix = %u|%I
              full_audit:success = open opendir
              full_audit:failure = all !open
              full_audit:facility = LOCAL7
              full_audit:priority = ALERT
    ```
    
    - Restart or reload samba server (**smbd**)
    
    ```
      service smbd restart  
    ```
  
  + Rsyslogd related 
    - Edit /etc/rsyslog.conf
    
    ```
      # *.* @hostname:<port>
      *.* @192.168.5.118:555
    ```
    
    - Restart or reload rsyslogd service
    
    ```
      service rsyslog restart
    ```

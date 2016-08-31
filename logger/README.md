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
      ...
      ...
      
      # *.* @hostname:<port>
      *.* @192.168.5.118:555
    ```
    
    - Restart or reload rsyslogd service
    
    ```
      # service rsyslog restart // not working
      ps -elf | grep rsyslog
      kill -9 UID
      service rsyslog start
    ```

### Usage
  + Host Platform: Window 7 64Bit (IP: 192.168.5.118)
  + Samba Server Platform: Ubuntu 16.04 64Bit (IP: 192.168.5.180)
  + Software: UDP/TCP testing tool
  + Method:
    - Open UDP/TCP testing tool
    - Create UDP testing
    - Listening on port 555
    - Visit Samba Server with 192.168.5.180
    - Check UDP/TCP testing tool's output window

# A Simple HttpServer
#### Author: Zhang Hao
#### Email: zhanghaohit@gmail.com

Test

### Supported Features:
+ GET method: load static HTML pages/files
+ Keep-alive feature
+ Disk file updates take effect immediately without restarting the server
+ Server-side caching

### Highlights:
+ Multithreaded server
+ Epoll-based event-triggered network handling so that it can handle a huge number of cocurrent connections effeciently.
+ Less CPU consumption when idle

### Usage:
#### 1. Compile
    (in the src directory)
    make
    (then it will generate a server and client executable files)
    
#### 2. Run
#### 1) server: the http server
     (you can get the available options by ./server --help)
     ./server
     [--port port (default: 12345)]
     [--bind_addr (default: )]
     [--threads threads (default: 3)]
     [--connections supported_max_connections (default: 10000)]
     [--root_dir root_dir (default: ./)]

#### 2) client: a simple client used to test the performance of the server
     (you can get the available options by ./client --help)
     ./client
     [--ip ip_address (default: localhost)]
     [--port port (default: 12345)]
     [--connections supported_max_connections (default: 10000)]
     [--sockets num_sockets (default: 100)]
  
#### 3) demo website
     (start the http server)
     ./server --root_dir ../public_html
     
     (access the website)
     open a web brower, then type the url: http://localhost:12345
     
 ### Some configuration on the max number of open file descriptors supported by the system
 #### 1) System-level max number of open file descriptors
     #The number of concurrently open file descriptors throughout the system
     cat /proc/sys/fs/file-max
     
     #if it is too small, edit the /etc/sysctl.conf by adding this
     sfs.file-max = 100000
     #make this take effect
     sudo sysctl -p
#### 2) User-level FD limits
    #check the hard limit
    ulimit -Hn
    #check the soft limit
    ulimit -Sn
    
    #if they are too small, edit /etc/security/limits.conf by putting 
    * soft nofile 20000
    * hard nofile 20000
    #to make this take effect without rebooting, edit the /etc/pam.d/common-session by adding
    session required pam_limits.so
    #then login by su username, it will take effect
    su username
     

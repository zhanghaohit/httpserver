# A Simple HttpServer
#### Author: Zhang Hao
#### Email: zhanghaohit@gmail.com


### Supported Features:
+ GET method: load static HTML pages/files
+ Keep-alive feature
+ Disk file updates take effect immediately without restarting the server
+ Server-side caching

### Highlights:
+ Multithreaded server
+ Epoll-based event-triggered network handling so that it can handle a huge number of cocurrent connections effeciently.

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

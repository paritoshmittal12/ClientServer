# <p align="center"> Client Server Socket Programming </p>

A **C** based multiple client and single server application that uses TCP to establish connection between Client and Server, and UDP to transfer data between the two.

The socket programming code has two main parts:
### **Server Side**
The C code takes as parameter the port to which it binds itself and starts to listen. The server waits in an infinite while loop for the connections. Every time a client initiates the request servers **forks** into a new child process. The child then opens the UDP communication, i.e. it switches from TCP to UDP. Initial process continues to listen for new connections and hence multiple clients are simultaneously handled properly.

```
 $  gcc -o server server.c
 $  sudo ./server 800 (Any other Port Number) 
```

### **Client Side**
The C code takes as parameter the ip address and port number of the server, it wishes to connect to. The application then tries to establish a connection and exchanges *Type 1* data using TCP. Once connection is establishes, it switches to UDP and asks the user for string it wishes to send. The string packet is then send to Server using UDP and connection is dropped.

```
 $ gcc -o client client.c 
 $ ./client ip_address port_number
```
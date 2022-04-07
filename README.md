# vk_server-client
A simple (it was, but not anymore) server-client solution made for a selection for VK internship.

In order to compile **everything** you can just run in Terminal `make` or `make all` in repository folder.  
To compile just a **server** or a **client** you should run 
`make server` 
or 
`make client`  

To execute server you should run:  
```
./server port folder/to/save/file
```
  
Port should be numerical and a folder should already exist.
Server has been written asynchronously so you can send multiple files from multiple clients at the same time.

To send a file to server you should run a client as  
```
./client server_address:port path/to/file
```
Server address could be in any format (as long as getaddrinfo() returns something useful) and a port should numerical. File to send should be a regular, normal file with appropriate permissions.

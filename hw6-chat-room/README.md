# HW6 - Chat Room

### How to run server:
```
./server -p PORT_NUMBER
```
Note: the -p flag is optional, default port is 13000

### How to run client: 
```
./client -p PORT_NUMBER -u USERNAME
```
Note: the -p flag is optional, default port is 13000, but the -u flag is mandatory

The client used here is not the same as the one from the previous assignment.

### What Doesn't Work:
The most obvious thing is that there is no signoff message. 
Aside from that, everything seems to work as expected.               

My original plan was to use pthread_cleanup_push and pthread_cleanup_pop, but that did not work as expected because the client didn't use threading. I found out about the atexit() function, but atexit() doesn't allow the function to be passed an argument, so I couldn't pass it the client's username and the file descriptor to write to the server in order to sign off. 
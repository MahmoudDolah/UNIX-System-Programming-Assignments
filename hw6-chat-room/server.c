#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define PORTNO 13000

int main(int argc, char* argv[]){
	long portnum = PORT_NUM;
	char* endptr;
	//remoteSocket = FD_NULL;
	remoteSocket = -1;
	// We should close the remote connection so that the remote end does
	// not end up with a socket stuck in TIME_WAIT.
	
	//atexit(closeRemoteConnection);

	// This is where the program will connect to/bind to.
	struct sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;


	clientSockets = calloc(MAX_CLIENTS, sizeof(int));
	if (clientSockets == NULL) {
		perror("Error at allocating memory for client sockets\n");
		exit(1);
	}

	//parseArguments(argc, argv, &PROG_NAME, &socketAddress, &DEBUG);
	for (int i = 0; i < argc; i++) {
		if (argv[i][1] == 'p') {
			portnum = strtol(argv[i+1], &endptr, 10);
		}
		else if (argv[i][1] == 'h'){
			printf("Usage: ./server -p PORT_NUMBER\n");
			printf("Note that the -p flag is optional (default port is 13000)\n");
			exit(0);
		}
	}
	printf("%li\n", portnum);
	if (portnum == PORT_NUM){
		printf("Using default port 13000\n");
	}
}
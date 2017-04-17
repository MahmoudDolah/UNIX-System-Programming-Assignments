#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define PORTNO 13000

int setupServer(int portnum);
void sendMessage(int sockfd, char* buffer);
void setupChat(int clientSockfd);

int main(int argc, char *argv[]){
	int portnum = PORTNO;
    char* endptr;

	for (int i = 0; i < argc; i++){
        if (argv[i][0] != '-'){
            continue;
        }
        else {
            if (argv[i][1] == 'p'){
                portnum = strtol(argv[i+1], &endptr, 10);
            }
            else {
                printf("usage: ./server -p PORT_NUMBER\n");
            }
        }
    }
    printf("portnum: %d\n", portnum);
    int clientSockfd = setupServer(portnum);
    printf("clientSockfd: %d\n", clientSockfd);
}

void setupChat(int clientSockfd){
	fd_set fdSET;
	int result = -1;
	FD_ZERO(&fdSET);
	FD_SET(clientSockfd, &fdSET);
	while (result == -1){
		result = select(clientSockfd+1, &fdSET, NULL, NULL, NULL);
	}

	if (result > 0){
		if (FD_ISSET(clientSockfd, &fdSET)){

		}
	}
}

void readMessage(int sockfd, char* buffer){
    if (read(sockfd, buffer, strlen(buffer) < 0)){
        perror("Error reading from client");
    }
}

void sendMessage(int sockfd, char* buffer){
    if (write(sockfd, buffer, strlen(buffer)) < 0){
        perror("Error writing to client");
    }
}

int setupServer(int portnum){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("Error creating socket");
    }

    struct sockaddr_in servAddr, clientAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(portnum);
    socklen_t clientLen = sizeof(clientAddr);
    if (bind(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
        perror("Error on binding\n");
        exit(1);
    }
    listen(sockfd, 1);
    int clientSockfd = accept(sockfd, (struct sockaddr *) &clientAddr, &clientLen);
    if (clientSockfd < 0){
        perror("Error on accepting client");
    }
    //close(sockfd);
    return clientSockfd;
}

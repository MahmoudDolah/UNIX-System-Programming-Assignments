#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdbool.h>

//char *PROG_NAME;
//bool DEBUG = false;

const int MAX_CLIENTS = 1;
const int MESSAGE_BUFSIZE = 4096;
const int USERNAME_BUFSIZE = 4096;
const int PORT_NUM = 13000;

char *SEPARATOR = ": ";
size_t SEPARATOR_LENGTH = 2;

static int remoteSocket;

void connectToServer(struct sockaddr_in *socketAddress, int *remotesocket);
void closeRemoteConnection();

int writeToFile(int file, char *message, size_t chars);

int main(int argc, char **argv) {
	remoteSocket = -1;
	atexit(closeRemoteConnection);

	char *username = malloc(sizeof(char) * USERNAME_BUFSIZE);
	if (username == NULL) {
		perror("username is null");
		exit(1);
	}

	long portnum = PORT_NUM;
	char* endptr;
	if (argc == 1) {
		printf("Usage: ./client -p PORT_NUMBER -u USERNAME\n");
		printf("Note that the -p flag is optional, but the -u flag is mandatory\n");
		printf("The default port is 13000\n");
		exit(1);
	}
	for (int i = 0; i < argc; i++) {
		if (argv[i][1] == 'p') {
			portnum = strtol(argv[i + 1], &endptr, 10);
		}
		else if (argv[i][1] == 'u') {
			username = argv[i + 1];
		}
		else if (argv[i][1] == 'h') {
			printf("Usage: ./client -p PORT_NUMBER -u USERNAME\n");
			printf("Note that the -p flag is optional, but the -u flag is mandatory\n");
			printf("The default port is 13000\n");
			exit(0);
		}
	}
	struct sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = INADDR_ANY;
	socketAddress.sin_port = htons(portnum);

	connectToServer(&socketAddress, &remoteSocket);

	int chars;
	char *message = malloc(sizeof(char) * MESSAGE_BUFSIZE);
	if (message == NULL) {
		perror("No message");
		exit(1);
	}

	size_t usernameLength = strlen(username);
	char *outmessage = malloc(sizeof(char) * (MESSAGE_BUFSIZE + usernameLength + SEPARATOR_LENGTH));
	if (outmessage == NULL) {
		perror("No out message");
		exit(1);
	}

	strncpy(outmessage, username, usernameLength);
	strncpy(outmessage + usernameLength, SEPARATOR,
	        SEPARATOR_LENGTH);

	fd_set dataSourceFds;
	int dataSourceFdsCount = remoteSocket + 1;
	int inputFds[] = {0, remoteSocket, -1};
	int outputFds[] = {remoteSocket, 1, -1};
	do {
		FD_ZERO(&dataSourceFds);
		FD_SET(0, &dataSourceFds);
		FD_SET(remoteSocket, &dataSourceFds);

		int selected = select(dataSourceFdsCount, &dataSourceFds, NULL, NULL, NULL);
		if (selected < 0) {
			perror("Select failed");
			exit(1);
		}

		for (int *in = inputFds, *out = outputFds;
		        *in != -1;
		        ++in, ++out) {

			if (FD_ISSET(*in, &dataSourceFds)) {
				chars = read(*in, message, MESSAGE_BUFSIZE);
				message[chars] = '\0';
				if (*out != 1) {
					strncpy(outmessage + usernameLength + SEPARATOR_LENGTH,
					        message, MESSAGE_BUFSIZE);

					size_t outmessageLength = usernameLength +
					                          SEPARATOR_LENGTH + chars;
					outmessage[outmessageLength] = '\0';

					if (writeToFile(*out, outmessage, outmessageLength) != 0) {
						perror("Can't write to file");
						exit(1);
					}
				} else {
					if (writeToFile(*out, message, chars) != 0) {
						perror("Write failed");
						exit(1);
					}
					fputs("\n", stdout);
				}
			}
		}
	} while (chars > 0);
	free(outmessage);
	free(message);
	return 0;
}

int writeToFile(int file, char *message, size_t chars) {
	/*
	  Writes a message to the specified file.
	  Returns 0 on success, 1 on error.
	*/
	ssize_t bytesWritten = 0;
	while ((bytesWritten = write(file, message + bytesWritten, chars - bytesWritten))) {
		if (bytesWritten < 0) {
			return 1;
		}
	}
	return 0;
}

void connectToServer(struct sockaddr_in *socketAddress, int *remotesocket) {
	/*
	  Connect to the server.
	  socketAddress is the remote server to connect to.
	  remotesocket is the file descriptor used to communicate to/from the
	  server.
	*/
	*remotesocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*remotesocket < 0) {
		perror("socket not initialized");
		exit(1);
	}

	socklen_t addrlen = sizeof(*socketAddress);

	if (connect(*remotesocket, (struct sockaddr *)(socketAddress), addrlen) != 0) {
		perror("Can't connect to server");
		exit(1);
	}
}

void closeRemoteConnection() {
	/*
	  Close any remote connections before exiting.
	*/
	if (remoteSocket != -1) {
		if (close(remoteSocket) != 0) {
			perror("Can't close connection");
		}
	}
}
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
#include <pthread.h>

const int MAX_CLIENTS = 32;
const int MAX_NUM_MESSAGES = 256;
const int MAX_MESSAGE_LENGTH = 1024;
const long PORT_NUM = 13000;

struct message_queue_t {
	char **messages;
	int *senders;
	size_t pushOffset;
	size_t popOffset;
	size_t maxMessageSize;
	size_t maxNumMessages;

	pthread_mutex_t lock;
};

int *clientSockets;
pthread_mutex_t clientSocketMutex = PTHREAD_MUTEX_INITIALIZER;

char *SEPARATOR = ": ";
size_t SEPARATOR_LENGTH = 2;

static int remoteSocket;

static struct message_queue_t g_messageQueue;

void listenForClients(struct sockaddr_in socketAddress, int *clientSockets);
void closeRemoteConnection();
int *getNextUnusedSocket(int *begin, int *end);
void *handleConnection(void *args);
void *propagateMessages(void *args);
void startMessagePropagationThread(int *clientSockets);
void message_queue_init(struct message_queue_t *messageQueue, size_t bufferSize, size_t messageLength);
void message_queue_cleanup(struct message_queue_t *messageQueue);
void message_queue_put(struct message_queue_t *messageQueue, char *message, int sender);
void message_queue_get(struct message_queue_t *messageQueue, char *message, int *sender);
void nullifyTrailingWhitespace(char *string);


int main(int argc, char *argv[]) {
	long portnum = PORT_NUM;
	char* endptr;
	remoteSocket = -1;
	atexit(closeRemoteConnection);

	struct sockaddr_in socketAddress;

	clientSockets = calloc(MAX_CLIENTS, sizeof(int));
	if (clientSockets == NULL) {
		perror("Error at allocating memory for client sockets\n");
		exit(1);
	}
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
	if (portnum == PORT_NUM){
		printf("Using default port 13000\n");
	}
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = INADDR_ANY;
	socketAddress.sin_port = htons(portnum);
	message_queue_init(&g_messageQueue, MAX_NUM_MESSAGES, MAX_MESSAGE_LENGTH);

	startMessagePropagationThread(clientSockets);

	listenForClients(socketAddress, clientSockets);

	message_queue_cleanup(&g_messageQueue);
	free(clientSockets);
	return 0;
}


void *handleConnection(void *args) {
	/*
	  Handles a single client connection.
	  *args should be a int *fd
	  fd is a file descriptor used to communicate with a chat client.
	  It will messages that the client connection sends to us into the
	  circular queue g_messages.
	*/
	int *socket = (int *)(args);
	char messageBuf[256];
	const size_t messageBufsize = 256;

	int chars;
	while ((chars = read(*socket, messageBuf, messageBufsize)) > 0) {
		if (chars < 0) {
			perror("Error: Unable to read");
			pthread_exit(NULL);
		}
		messageBuf[chars] = '\0';
		message_queue_put(&g_messageQueue, messageBuf, *socket);
	}

	pthread_mutex_lock(&clientSocketMutex);
	if (close(*socket) < 0) {
		perror("Failed to close socket");
	}
	*socket = 0;
	pthread_mutex_unlock(&clientSocketMutex);
	pthread_exit(NULL);
}

void *propagateMessages(void *args) {
	/*
	  Sends messages received to clients.
	  *args should be a int *fd.
	  fd is an array of file descriptors, of length MAX_CLIENTS.
	*/
	int *sockets = (int *)args;
	int originalSender;

	char *message = calloc(MAX_MESSAGE_LENGTH, sizeof(char));
	if (message == NULL) {
		perror("Unable to allocate memory for message");
		exit(1);
	}

	while (true) {
		message_queue_get(&g_messageQueue, message, &originalSender);
		nullifyTrailingWhitespace(message);
		fprintf(stdout, "%s\n", message);

		pthread_mutex_lock(&clientSocketMutex);
		for (size_t i = 0; i < MAX_CLIENTS; ++i) {
			if (sockets[i] != 0 && sockets[i] != originalSender) {
				if (write(sockets[i], message, strlen(message)) < 0) {
					perror("Write failed");
				}
			}
		}
		pthread_mutex_unlock(&clientSocketMutex);
	}
	free(message);
	pthread_exit(NULL);
}


void listenForClients(struct sockaddr_in socketAddress, int *clientSockets) {
	/*
	  Waits for a client to connect to us.
	  socketAddress is the address where we should listen on.
	  clientSockets are file descriptors used to communicate to/from the
	  remote client.  This is treated as a circular buffer.
	  numClients are the maximum number of clients.
	  clientSocketMutex is used to control access to it.
	*/
	int serversocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serversocket < 0) {
		perror("No server socket");
	}

	socklen_t addrlen = sizeof(socketAddress);

	if (bind(serversocket, (struct sockaddr *)(&socketAddress), addrlen) != 0) {
		perror("Bind failed");
		exit(1);
	}

	if (listen(serversocket, MAX_CLIENTS) != 0) {
		perror("Listen failed");
		exit(1);
	}

	struct sockaddr remoteAddress;
	socklen_t remoteAddrLen = sizeof(remoteAddress);

	int *nextSocket = clientSockets;
	int *afterLastSocket = clientSockets + MAX_CLIENTS;

	pthread_t threadId;
	int pthreadErrno;

	while (true) {
		int acceptedSocket = accept(serversocket, &remoteAddress, &remoteAddrLen);
		if (acceptedSocket < 0) {
			perror("No accepted socket");
			exit(1);
		}

		pthread_mutex_lock(&clientSocketMutex);
		if ((nextSocket = getNextUnusedSocket(clientSockets, afterLastSocket)) == NULL) {
			close(acceptedSocket);
		} else {
			if ((pthreadErrno = pthread_create(&threadId, NULL, handleConnection, (void *)(nextSocket))) != 0) {
				fputs("Could not create new thread to handle request.", stderr);
			} else {
				*nextSocket = acceptedSocket;
			}
		}
		pthread_mutex_unlock(&clientSocketMutex);
	}
	close(serversocket);
}

int writeToFile(int file, char *message, size_t chars) {
	ssize_t bytesWritten = 0;
	while ((bytesWritten = write(file, message + bytesWritten, chars - bytesWritten))) {
		if (bytesWritten < 0) {
			return 1;
		}
	}
	return 0;
}

void connectToServer(struct sockaddr_in *socketAddress, int *remotesocket) {
	*remotesocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*remotesocket < 0) {
		perror("Socket doesn't exist");
		exit(1);
	}

	socklen_t addrlen = sizeof(*socketAddress);

	if (connect(*remotesocket, (struct sockaddr *)(socketAddress), addrlen) != 0) {
		perror("Cannot connect");
		exit(1);
	}

}

void closeRemoteConnection() {
	/*
	  Close any remote connections before exiting.
	*/
	if (remoteSocket != -1) {
		if (close(remoteSocket) != 0) {
			perror("Close failed");
		}
	}
}

int *getNextUnusedSocket(int *begin, int *end) {
	/*
	  Returns the next available socket position.

	  begin and end are the beginning and end positions of the socket array,
	  respectively.

	  Returns a pointer if one is found, and NULL otherwise.

	  Assumes that there is **no simultaneous access** to the socket array.
	  Be sure to lock it beforehand!
	*/
	for (; begin != end; ++begin) {
		if (*begin == 0) {
			return begin;
		}
	}
	return NULL;
}


void startMessagePropagationThread(int *clientSockets) {
	/*
	  Starts a thread that handles message propagation.
	*/
	pthread_t threadId;
	int pthreadErrno;

	if (pthread_create(&threadId, NULL, propagateMessages, (void *)(clientSockets)) != 0) {
		perror("Can't create thread");
		exit(1);
	}
}


void message_queue_init(struct message_queue_t *messageQueue, size_t bufferSize, size_t messageSize) {
	/*
	  Initializes a message queue.

	  bufferSize controls how many messages will fit into the queue at any
	  given time.

	  Memory is allocated to hold the messages.
	*/
	if (pthread_mutex_init(&messageQueue->lock, NULL) != 0) {
		perror("Pthread mutex wasn't initialized properly");
		exit(1);
	}

	messageQueue->messages = calloc(bufferSize, sizeof(char *));
	if (messageQueue->messages == NULL) {
		perror("No messages");
		exit(1);
	}

	messageQueue->senders = calloc(bufferSize, sizeof(int));
	if (messageQueue->senders == NULL) {
		perror("No senders");
		exit(1);
	}

	for (size_t i = 0; i < bufferSize - 1; ++i) {
		char *message = calloc(messageSize, sizeof(char));
		if (message == NULL) {
			perror("Nothing in message");
			exit(1);
		}
		messageQueue->messages[i] = message;
	}
	messageQueue->pushOffset = 0;
	messageQueue->popOffset = 0;

	messageQueue->maxMessageSize = messageSize;
	messageQueue->maxNumMessages = bufferSize;

}

void message_queue_cleanup(struct message_queue_t *messageQueue) {
	/*
	  Cleans up a message queue.

	  Frees up the internal memory buffer.

	  You should make sure that no one is using it anymore before cleaning
	  up.
	*/
	if (pthread_mutex_destroy(&messageQueue->lock) != 0) {
		perror("Can't destroy mutex");
		exit(1);
	}
	for (; *messageQueue->messages != NULL; ++messageQueue->messages) {
		free(*messageQueue->messages);
	}
	free(messageQueue->messages);
	free(messageQueue->senders);
}

void message_queue_put(struct message_queue_t *messageQueue, char *message, int sender) {
	/*
	  Puts a message into the message queue.

	  message is the message to store.
	  sender is a unique ID to identify sender is associated with the message.

	  May block if the queue is currently full.
	*/
	pthread_mutex_lock(&messageQueue->lock);

	while (messageQueue->pushOffset + 1 == messageQueue->popOffset) {
		pthread_mutex_unlock(&messageQueue->lock);
		pthread_yield();

		pthread_mutex_lock(&messageQueue->lock);
	}
	if (!strncpy(messageQueue->messages[messageQueue->pushOffset], message,
	             messageQueue->maxMessageSize)) {
		perror("Copy failed");
	}
	messageQueue->senders[messageQueue->pushOffset] = sender;
	messageQueue->pushOffset =
	    (messageQueue->pushOffset + 1) % messageQueue->maxNumMessages;
	pthread_mutex_unlock(&messageQueue->lock);
}

void message_queue_get(struct message_queue_t *messageQueue, char *message, int *sender) {
	/*
	  Gets a message and its sender from the message queue.

	  May block if the queue is currently empty.
	*/
	pthread_mutex_lock(&messageQueue->lock);

	while (messageQueue->popOffset == messageQueue->pushOffset) {
		pthread_mutex_unlock(&messageQueue->lock);
		pthread_yield();
		pthread_mutex_lock(&messageQueue->lock);
	}

	if (!strncpy(message, messageQueue->messages[messageQueue->popOffset],
	             messageQueue->maxMessageSize)) {
		perror("Error at line 649");
	}

	*sender = messageQueue->senders[messageQueue->popOffset];
	messageQueue->popOffset = (messageQueue->popOffset + 1) % messageQueue->maxNumMessages;

	pthread_mutex_unlock(&messageQueue->lock);
}

void nullifyTrailingWhitespace(char *string) {
	/*
	  Terminates a string at the first trailing whitespace character.
	*/
	char *lastValidChar = string;
	for (; *string != '\0'; ++string)
	{
		if (*string != ' ' && *string != '\t' && *string != '\n')
		{
			lastValidChar = string;
		}
	}

	for (++lastValidChar; *lastValidChar != '\0'; ++lastValidChar)
	{
		*lastValidChar = '\0';
	}
}

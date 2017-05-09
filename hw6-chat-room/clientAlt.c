#include <stdio.h>
#include <stdlib.h>

#define PORT_NUM 13000;


int main(int argc, char *argv[]) {
	long portnum = PORT_NUM;
	char* username = NULL;
	char* endptr;
	if (argc == 1){
		printf("Usage: ./client -p PORT_NUMBER -u USERNAME\n");
		printf("Note that the -p flag is optional, but the -u flag is mandatory\n");
		printf("The default port is 13000\n");
		exit(1);
	}
	for (int i = 0; i < argc; i++){
		if (argv[i][1] == 'p'){
			portnum = strtol(argv[i+1], &endptr, 10);
		}
		else if (argv[i][1] == 'u'){
			username = argv[i+1];
		}
		else if (argv[i][1] == 'h'){
			printf("Usage: ./client -p PORT_NUMBER -u USERNAME\n");
			printf("Note that the -p flag is optional, but the -u flag is mandatory\n");
			printf("The default port is 13000\n");
			exit(0);
		}
	}
	if (username == NULL){
		perror("Username not set\n");
		exit(1);
	}
	printf("port: %li, username: %s\n", portnum, username);
}
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define PORTNO 13000

int main(int argc, char* argv[]){
	int portnum = PORTNO;
	char* endptr;

	for (int i = 0; i < argc; i++){
		if (argv[i][1] == 'p'){
			portnum = strtol(argv[i+1], &endptr, 10);
		}
	}
	if (portnum == PORTNO){
		printf("The server is currently running on the default port, which is port 13000\n");
		printf("You can change this by using the -p flag\n");
	}
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORTNO 13000

int main(int argc, char *argv[]){
	if (argc == 1){
        //perror("Need the command line arguments\n");
	}

	int portnum = PORTNO;
	//printf("portnumber is %d \n", portnum);

    char* endptr;
    char* ip;

	for (int i = 0; i < argc; i++){
		//puts(argv[i]);
        if (argv[i][0] != '-'){
            continue;
        }
        else {
            if (argv[i][1] == 'p'){
                portnum = strtol(argv[i+1], &endptr, 10);
            }
            else if (argv[i][1] == 'i'){
                ip = argv[i+1];
            }
            else {
                printf("usage: ./server -i IP_ADDRESS -p PORT_NUMBER\n");
                printf("Both command line arguments are optional\n");
            }
        }
    }
    printf("portnum: %d\n", portnum);
    printf("ip: %s\n", ip);
}

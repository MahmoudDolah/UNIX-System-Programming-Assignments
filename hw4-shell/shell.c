#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<error.h>
#include<sys/stat.h>
#include<fcntl.h>

const int MAX_LEN = 200;
const int MAX_TOKENS = 50;

void loopShell();
void getCommand(char* prompt, char* buffer, int size);
void parseCommand(char* buffer);
void executeCommand(char* command[]);
void redirect(char** commands);

int main(int argc, char **argv){
	loopShell();
	return 0;
}

void getCommand(char* prompt, char* buffer, int size){
	/*
	Gets the command from the user input and puts it in buffer
	*/
	printf("%s", prompt);
	if (fgets(buffer, size, stdin) == NULL){
		printf("\n No Commands\n");
		exit(1);
	}
	if (buffer[strlen(buffer)-1] != '\n'){
		int charPos, tooLong;
		tooLong = 0;
		while (((charPos = getchar()) != '\n') && charPos != EOF){
			tooLong = 1;
		}
		if (tooLong){
			printf("Command too long\n");
			exit(1);
		}
	}
	buffer[strlen(buffer)-1] = '\0';
}

void executeCommand(char* command[]){
	/*
	Executes the command using execvp
	Waits until command is finished before returning to shell
	*/
	int pid = fork();
	if (pid == 0){
		//redirect(command);
		execvp(command[0], command);
		perror("Unable to execute command\n");
		exit(1);
	}
	else{
		wait(NULL);
	}
}

void redirect(char* commands[]){
	/*
	Handles IO Redirection
	Takes in array of the commands
	*/
    int hitInput = 0;
    int hitOutput = 0;
    char input[64];
    char output[64];
    for (int i = 0; commands[i] != '\0'; i++){
        if (strcmp(commands[i], '>')){
            hitInput = 1;
            strcpy(input, commands[i+1]);
        }
        if (strcmp(commands[i], '<')){
            hitOutput = 1;
            strcpy(output, commands[i+1]);
        }
    }
    if (hitInput == 1){
        int fd0;
        if ((fd0 = open(input, O_RDONLY, 0)) < 0){
            perror("Can't open file\n");
            exit(0);
        }
        dup2(fd0, 0);
        close(fd0);
    }
    if (hitOutput == 1){
        int fd1;
        if ((fd1 = open(output, 0644)) < 0){
            perror("Can't open the output file\n");
            exit(0);
        }
        dup2(fd1, 1);
        close(fd1);
    }
}

void parseCommand(char* buffer){
	/*
	Parses the input
	Takes in a buffer of the input, uses strtok
	to tokenize and then executes it
	*/
	if (!strcmp(buffer, "exit")){
		exit(0);
	}
	char* sep = " ";
	char* token = strtok(buffer, sep);
	if (token == NULL){
		perror("No commands\n");
		exit(1);
	}
	int tokenCount = 0;
    char* commands[MAX_TOKENS];
    commands[0] = token;
    while (token != NULL){
		token = strtok(NULL, sep);
		commands[++tokenCount] = token;
    }
	if (!strcmp(commands[0], "cd")){
		chdir(commands[1]);
		return;
	}

	executeCommand(commands);
}

void loopShell(){
    /*
    Starts the shell
    Loops infinitely to get commands from user
    */
	char* prompt = getenv("PS1");
	if (!prompt){
		prompt = "h4ck-th3-p14n3t: $ ";
	}
	while (1){
		char buffer[MAX_LEN];
		getCommand(prompt, buffer, MAX_LEN);
		parseCommand(buffer);
	}
}

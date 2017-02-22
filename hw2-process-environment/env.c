#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

extern const char** environ;

void display();
void runCommand(char** commands);
void replaceEnviron(char** commands, char** nameVal);

int main (int argc, char* argv[]){
    int startNameVal = 1;
    int endNameVal = 1;
    int iFlag = 0;
    int isCommand = 0;
    int enter = 0;
    if (argc == 1){
        display();
        exit(0);
    }
    if (strcmp(argv[1], "-i") == 0){
        iFlag = 1;
        startNameVal = 2;
        endNameVal = 2;
    }
    for (int i = 1; i < argc; i++){
        if ((iFlag == 1) & (enter == 0)){
            enter = 1;
        }
        else{
            endNameVal = i;
            if (strpbrk(argv[endNameVal], "=") != 0){
                endNameVal++; // Count total number of commands
            }
        }
    }
    char* nameVal[(endNameVal+1)-startNameVal];
    int numNameVal = 0;
    for (int i = startNameVal; i < endNameVal; i++){
        nameVal[numNameVal] = argv[i];
        numNameVal++;
    }
    nameVal[numNameVal] = NULL;
    char* commands[argc - endNameVal];
    int k = 0;
    for (int i = endNameVal; i < argc; i++){
        commands[k] = argv[i];
        k++;
    }
    if (k > 0){
        isCommand = 1;
    }

    if ((iFlag == 0) && (isCommand == 0)){
        display();
    }
    else if ((iFlag == 0) && (isCommand == 1)) {
        runCommand(commands);
    }
    else if ((iFlag == 1) && (isCommand == 1)){
        replaceEnviron(commands, nameVal);
    }
}
/*
void addToEnviron(char** commands, char** nameVal, int numNameVal){
    char* envName;
    char* envVal;
    char* name;
    char* val;
    int overwrite = 0;
    for (int i = 0; environ[i] != NULL; ++i){
        for (int j = 0; j < numNameVal; ++j){
            envName = strtok(environ[i], "=");
            val = strtok(nameVal[i], "=");
            envName = strtok(environ[i], "=");
            val = strtok(nameVal[i], "=");
            if (strcmp(val, envName)){
                if (strcmp(envVal, val)){

                }
            }
        }
    }
}
*/
void replaceEnviron(char** commands, char** nameVal){
    if (execvp(*commands, nameVal) < 0){
        perror("execvp in replaceEnviron() function failed\n");
    }
}

void runCommand(char** commands){
    if (execvp(*commands, environ) < 0){
        perror("execvp in runCommand() function failed\n");
    }
}

void display(){
    for (int i = 0; environ[i] != NULL; ++i){
        puts(environ[i]);
    }
}

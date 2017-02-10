#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

extern char** environ;

void display();
void displayChanges(char* commands);

int main (int argc, char* argv[]){
    //char* nameVal;
    //char* cmd[5];
    //char* arg[5];
    int startNameVal = 0;
    int endNameVal = 0;
    int iFlag = 0;
    int isCommand = 0;
    int enter = 0;
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i-1], "-i") == 0){
            iFlag = 1;
        }
        if ((iFlag == 1) & (enter == 0)){
            startNameVal = i;
            enter = 1;
        }
        if (iFlag == 1){
            endNameVal = i;
            if (strpbrk(argv[endNameVal], "=") != 0){
                //argv[j];
                endNameVal++; // Count total number of commands
                // Causes seg fault bc checks over edge
            }
        }
    }
    char* nameval[(endNameVal)-startNameVal];
    int j = 0;
    for (int i = startNameVal; i < endNameVal-1; i++){
        nameval[j] = argv[i];
        j++;
    }
    if (endNameVal < argc){
        isCommand == 1;
    }
    char* commands[argc - (endNameVal-1)];
    int k = 0;
    for (int i = (endNameVal-1); i < argc; i++){
        commands[k] = argv[i];
        k++;
    }
    /*
    printf("startNameVal: %d, endNameVal: %d\n", startNameVal, endNameVal);
    printf("All of argv\n");
    for (int i = 0; i < argc; i++){
        printf("%s\n", argv[i]);
    }
    printf("Just path part of argv\n");
    for (int i = (startNameVal-1); i < endNameVal; i++){
        printf("%s\n", argv[i]);
    }
    printf("nameval array\n");
    for (int i = 0; i < (endNameVal - startNameVal); i++){
        printf("%s\n", nameval[i]);
    }
    printf("commands array\n");
    for (int i = 0; i < k; i++){
        printf("%s\n", commands[i]);
    }
    */
    if ((iFlag == 0) && (isCommand == 0)){
        display();
    }
    else if ((iFlag == 0) && (isCommand == 1)) {
        displayChanges(commands);
    }
}

void displayChanges(char* commands){
    execv(commands);
}

void display(){
    for (char* c = *environ; c; c = *++environ){
        puts(c);
    }
}

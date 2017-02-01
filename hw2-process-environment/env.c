#include<stdio.h>
#include<stdlib.h>
#include<string.h>

extern char** environ;

void display();

int main (int argc, char** argv){
    char* nameVal;
    char* cmd;
    char* arg;
    if (argc > 1){
        for (int i = 1; i < argc; i++){
            if (i == 2){
                nameVal = argv[i];
            }
            else if (i == 3){
                cmd = argv[i];
            }
            else if (i == 4){
                arg = argv[i];
            }
        }
    }
    display();
}

void display(){
    for (char* c = *environ; c; c=*++environ){
        puts(c);
    }
    /*
    for (char** p = environ; *p; p++){
        puts(*p);
    }
    */
}

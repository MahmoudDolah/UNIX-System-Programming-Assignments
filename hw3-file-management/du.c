#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<dirent.h>
#include<unistd.h>

int main (int argc, char* argv[]){
    char* dir;
    getcwd(dir, sizeof(dir));
    if (argc == 2){
        dir = argv[1];
    }
    printf("%s\n", dir);
}

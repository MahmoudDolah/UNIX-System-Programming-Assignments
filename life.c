#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char**argv)
{
    printf("The Game of Life\n");
    // Default values for command line arguments
    int rows = 10;
    int columns = 10;
    int generations = 10;
    char* filename = "life.txt";
    char *ptr;

    for (int i = 1; i < argc; i++){
        if (i == 1){
            rows = strtol(argv[i], &ptr, 10);
        }
        else if (i == 2){
            columns = strtol(argv[i], &ptr, 10);
        }
        else if (i == 3){
            filename = argv[i];
        }
        else if (i == 4){
          generations = strtol(argv[i], &ptr, 10);
        }
    }
/*
    printf("Rows: %d\n", rows);
    printf("Columns: %d\n", columns);
    printf("filename: %s\n", filename);
    printf("Generations: %d\n", generations);
*/
}

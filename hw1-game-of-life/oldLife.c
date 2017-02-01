#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct cell{
    struct cell* neighbors[8];
    int alive;
} cell;

typedef struct world{
    cell** positions;
    int row;
    int column;
    void* mem;
} world;

void createWorld(world* theWorld, int rows, int columns);
void inputWorld(char* filename, world* theWorld, int rows, int columns);
void printWorld(world* theWorld);

int main(int argc, char** argv){
    // Default values for command line arguments
    int rows = 10;
    int columns = 10;
    int generations = 10;
    char *filename = "life.txt";
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
    world gameoflife;
    /*
    gameoflife.column = columns;
    gameoflife.row = rows;
    gameoflife.positions[rows];
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < columns; j++){
            gameoflife.positions[i][columns];
            // gameoflife.positions[i][j].alive = 0;
        }
    }
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < columns; j++){
            gameoflife.positions[i][j].alive = 0;
        }
    }
    */
    createWorld(&gameoflife, rows, columns);
    inputWorld(filename, &gameoflife, rows, columns);
    printWorld(&gameoflife);
}

void createWorld(world* theWorld, int rows, int columns){
    long height = sizeof(cell *) * columns;
    long width = sizeof(cell *) * rows;
    theWorld->mem = malloc(height + (width * columns));


}

void inputWorld(char* filename, world* theWorld, int rows, int columns){
    FILE* fp;
    fp = fopen(filename, "r");
    size_t read;
    size_t len = 3;
    char* line = NULL;

    for (int k = 0; k < rows; k++){
        while ((read = getline(&line, &len, fp)) != -1){
            for (int i = 0; i < strlen(line); i++){
                if (strcmp(line[i], "*")){
                    theWorld->positions[k][i].alive = 1;
                }
            }
        }
    }
    fclose(fp);
}

void printWorld(world* theWorld){

    for (int i = 0; i < sizeof(theWorld->positions); i++){
        for (int j = 0; j < sizeof(theWorld->positions[i]); j++){
            if (theWorld->positions[i][j].alive == 1){
                printf("*");
            }
            else{
                printf(" ");
            }
        }
    }
}

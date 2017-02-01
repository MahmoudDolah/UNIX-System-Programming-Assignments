#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct cell{
    struct cell* neighbors[8]; // The 8 cells that surround it
    int alive;
} cell;

typedef struct world{
    cell** positions;
    int rows;
    int columns;
    void* mem;
} world;

void createWorld(world* gameoflife, int rows, int columns);
void initializeWorld(const char* filename, world* gameoflife, int rows, int columns);
void updateWorld(world* gameoflife);
void printWorld(world* gameoflife);
void destroyWorld(world* gameoflife);

int main (int argc, char** argv){
    int rows = 10;
    int columns = 10;
    int generations = 10;
    char *filename = "life.txt";
    char *ptr;
    world gameoflife;

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

    createWorld(&gameoflife, rows, columns);
    initializeWorld(filename, &gameoflife, rows, columns);
    for (int i = 0; i < generations; i++){
        updateWorld(&gameoflife);
        printWorld(&gameoflife);
    }
    destroyWorld(&gameoflife);
}
void destroyWorld(world* gameoflife){
    free(gameoflife->mem);
}

void printWorld(world* gameoflife){
    for (int i = 0; i < gameoflife->columns; i++){
        for (int j = 0; j < gameoflife->rows; j++){
            if (gameoflife->positions[i][j].alive == 1){
                printf("*");
            }
            else if (gameoflife->positions[i][j].alive == 0){
                printf(" ");
            }
        }
    }
}

void updateWorld(world* gameoflife){
    int row, column, counter, neighbors;
    // row and column are positions
    // counter is a counter for the neighbors next to a cell

    for (row = 0; row < gameoflife->rows; row++){
        for (column = 0; column < gameoflife->columns; column++){
            for (counter = 0; counter < 8; counter++){
                if ((gameoflife->positions[row][column].neighbors[counter]) && ((gameoflife->positions[row][column]).neighbors[counter]->alive == 1)){
                    neighbors++;
                }
            }
            if (gameoflife->positions[row][column].alive == 1){
                if (neighbors < 2){
                    gameoflife->positions[row][column].alive = 0;
                }
                else if(neighbors == 3){
                    gameoflife->positions[row][column].alive = 0;
                }
            }
            else if (gameoflife->positions[row][column].alive == 0){
                if (neighbors == 3){
                    gameoflife->positions[row][column].alive = 1;
                }
            }
        }
    }

}

void initializeWorld(const char* filename, world* gameoflife, int rows, int columns){
    /* Takes input from file and puts it into the gameoflife array
    */
    FILE* fp;
    fp = fopen(filename, "r");
    char* read = NULL;

    for (int i = 0; i < columns; i++){
        char * line = fgets(read, rows, fp);
        for (int j = 0; j < rows; j++){
            if (strcmp(&line[j], "\n")){
                break;
            }
            else if (strcmp(&line[j], "*")){
                gameoflife->positions[i][j].alive = 1;
            }
        }
    }
    fclose(fp);
}

void createWorld(world* gameoflife, int rows, int columns){
    int i, j; // i and j are the coordinates on the board
    unsigned long length = sizeof(cell *) * columns;
    unsigned long height = sizeof(cell) * rows;

    gameoflife->mem = malloc(length + (height * columns));

    gameoflife->positions  = gameoflife->mem;
    gameoflife->rows  = rows;
    gameoflife->columns = columns;

    for(i = 0; i < columns; i++) {
       gameoflife->positions[i] = gameoflife->mem + length + (i * height);
    }

    for(i = 0; i < columns; i++) {
        for(j = 0; j < rows; j++) {
            if(j != 0) {
                (gameoflife->positions[i][j]).neighbors[3] = &(gameoflife->positions[i][j - 1]);
            }

            if(i != 0) {
                (gameoflife->positions[i][j]).neighbors[1] = &(gameoflife->positions[i - 1][j]);
            }

            if(j != (rows - 1)) {
                (gameoflife->positions[i][j]).neighbors[4] = &(gameoflife->positions[i][j + 1]);
            }

            if(i != (columns - 1)) {
                (gameoflife->positions[i][j]).neighbors[6] = &(gameoflife->positions[i + 1][j]);
            }

            if((i != 0) && (j != 0)) {
                (gameoflife->positions[i][j]).neighbors[0] = &(gameoflife->positions[i - 1][j - 1]);
            }

            if((i != (columns - 1)) && (j != (rows - 1))) {
                (gameoflife->positions[i][j]).neighbors[7] = &(gameoflife->positions[i + 1][j + 1]);
            }

            if((i != (columns - 1)) && (j != 0)) {
                (gameoflife->positions[i][j]).neighbors[5] = &(gameoflife->positions[i + 1][j - 1]);
            }

            if((i != 0) && (j != (rows - 1))) {
                (gameoflife->positions[i][j]).neighbors[2] = &(gameoflife->positions[i - 1][j + 1]);
            }
        }
    }
}

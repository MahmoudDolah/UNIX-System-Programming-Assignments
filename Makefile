FLAGS = -Wall -std=c99 -o life

all: 
    gcc life.c ${FLAGS}

clean:
    rm life
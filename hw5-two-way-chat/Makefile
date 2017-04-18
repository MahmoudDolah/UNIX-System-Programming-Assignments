#Makefile
FLAGS = -std=c99 -Wall

all: client server

client: client.c
	gcc ${FLAGS} -o client client.c

server: server.c
	gcc ${FLAGS} -o server server.c

clean:
	rm -f server client core

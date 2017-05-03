#Makefile
FLAGS = -std=c99 -Wall

all: client server

server: server.c
	gcc ${FLAGS} -pthread -o server server.c

client: client.c
	gcc ${FLAGS} -o client client.c

clean:
	rm -f server client core

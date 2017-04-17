#Makefile
flags = -std=c99 -Wall -01

all: client server

client: client.c
	gcc ${FLAGS} -o client client.c

server: server.c
	gcc ${FLAGS} -o server server.c

clean:
	rm -f server client core

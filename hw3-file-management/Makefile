#Makefile
FLAGS = -Wall

all: du du_nftw

du: du.c
	gcc ${FLAGS} -o du du.c

du_nftw: du_nftw.c
	gcc ${FLAGS} -o du_nftw du_nftw.c

clean:
	rm -rf du du_nftw core

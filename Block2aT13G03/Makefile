
CC = gcc

CFLAGS  = -g -Wall



all :  clie serv

clie : client.c
	$(CC) $(CFLAGS) -o client client.c

serv: server.c	
	$(CC) $(CFLAGS) -o server server.c
	
clean:
	rm server
	rm client




	
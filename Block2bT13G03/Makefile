
CC = gcc

CFLAGS  = -g -Wall



all :  clie serv clieup clienthttp 

clie : client.c
	$(CC) $(CFLAGS) -o client client.c

serv: server.c	
	$(CC) $(CFLAGS) -o server server.c

clieup:clientudp.c
	$(CC) $(CFLAGS) -o clientudp clientudp.c

clienthttp: 
	$(CC) $(CFLAGS) -o clienthttp clienthttp.c
	
clean:
	rm server
	rm client




	
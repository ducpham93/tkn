
CC = gcc

CFLAGS  = -g -Wall  -std=c99 
ARGS1 = 190 127.0.0.1 4190 10 127.0.0.1 4010 70 127.0.0.1 4070
ARGS2 = 10 127.0.0.1 4010 70  127.0.0.1 4070 130 127.0.0.1 4130
ARGS3 = 70 127.0.0.1 4070 130 127.0.0.1 4130 190 127.0.0.1 4190
ARGS4 = 130 127.0.0.1 4130 190 127.0.0.1 4190 10 127.0.0.1 4110

ARGSSET = localhost 4010 SET TKN70 abc123
ARGSGET =  localhost 4010 GET TKN70
ARGSDELETE= localhost 4010 DELETE TKN70


#########compiling###########
all:   per clie

per: peer.c	
	$(CC) $(CFLAGS) -o peer peer.c hashtabelle.c

clie: client.c	
	$(CC) $(CFLAGS) -o client client.c	

#########run###########


run1: peer 
	./peer  ${ARGS1}
run2: peer 
	./peer  ${ARGS2}
run3: peer 
	./peer  ${ARGS3}
run4: peer 
	./peer  ${ARGS4}

runset: client 
	./client  ${ARGSSET}

runget: client 
	./client  ${ARGSGET}


rundelete: client 
	./client  ${ARGSDELETE}

	
clean:
	rm peer
	rm client

	




	
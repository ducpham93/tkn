
CC = gcc

CFLAGS  = -g -Wall  -std=c99 
ARGS1 = 10 127.0.0.1 4010 190 127.0.0.1 4190  70 127.0.0.1 4070
ARGS2 = 70 127.0.0.1 4070 10  127.0.0.1 4010  130 127.0.0.1 4130
ARGS3 = 130 127.0.0.1 4130 70 127.0.0.1 4070  190 127.0.0.1 4190
ARGS4 = 190 127.0.0.1 4190 130 127.0.0.1 4130  10 127.0.0.1 4010

# ARGS1 = 10 127.0.0.1 4010 130 127.0.0.1 4130  70 127.0.0.1 4070
# ARGS2 = 70 127.0.0.1 4070 10  127.0.0.1 4010  130 127.0.0.1 4130
# ARGS3 = 130 127.0.0.1 4130 70 127.0.0.1 4070  10 127.0.0.1 4010
# ARGS4 = 190 127.0.0.1 4190 130 127.0.0.1 4130  10 127.0.0.1 4010


ARGSSET = localhost 4070 SET TKN70 abc123
ARGSGET =  localhost 4070 GET TKN70
ARGSDELETE= localhost 4070 DELETE TKN70



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

	




	
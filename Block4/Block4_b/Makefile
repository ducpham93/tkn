
CC = gcc

CFLAGS  = -g -Wall  -std=c99 
ARGS1 = 10 127.0.0.1 4010 190 127.0.0.1 4190  70 127.0.0.1 4070
ARGS2 = 70 127.0.0.1 4070 10  127.0.0.1 4010  130 127.0.0.1 4130
ARGS3 = 130 127.0.0.1 4130 70 127.0.0.1 4070  190 127.0.0.1 4190
ARGS4 = 190 127.0.0.1 4190 130 127.0.0.1 4130  10 127.0.0.1 4010


# ARGS1 = 10 127.0.0.1 4010 130 127.0.0.1 4130  70 127.0.0.1 4070
# ARGS2 = 70 127.0.0.1 4070 10  127.0.0.1 4010  130 127.0.0.1 4130
# ARGS3 = 130 127.0.0.1 4130 70 127.0.0.1 4070  10 127.0.0.1 4010
# ARGS1 = 10 localhost 4010 130 localhost 4130  70 localhost 4070
# ARGS2 = 70 localhost 4070 10  localhost 4010  130 localhost 4130
# ARGS3 = 130 localhost 4130 70 localhost 4070  10 localhost 4010
# ARGS4 = 190 127.0.0.1 4190 130 127.0.0.1 4130  10 127.0.0.1 4010


ARGSSET = localhost 4070 SET TKN70 abc123
ARGSGET =  localhost 4070 GET TKN70
ARGSDELETE= localhost 4070 DELETE TKN70


ARGSALONE= 127.0.0.1 4130 130
ARGSJOIN1 = 127.0.0.1 4010 10 127.0.0.1 4130 130
ARGSJOIN2 = 127.0.0.1 4070 70 127.0.0.1 4130 130
ARGSJOIN3 =  127.0.0.1 4190 190 127.0.0.1 4070 70



#########compiling###########,/
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

runalone: peer 
	./peer  ${ARGSALONE}

runjoin1: peer 
	./peer  ${ARGSJOIN1}

runjoin2: peer 
	./peer  ${ARGSJOIN2}

runjoin3: peer 
	./peer  ${ARGSJOIN3}

	
clean:
	rm peer
	rm client

	




	
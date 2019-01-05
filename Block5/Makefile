CC = gcc

CFLAGS  = -g -Wall  -std=c99 
ARGS1 = 10 127.0.0.1 4010 190 127.0.0.1 4190  70 127.0.0.1 4070
ARGS2 = 70 127.0.0.1 4070 10  127.0.0.1 4010  130 127.0.0.1 4130
ARGS3 = 130 127.0.0.1 4130 70 127.0.0.1 4070  190 127.0.0.1 4190
ARGS4 = 190 127.0.0.1 4190 130 127.0.0.1 4130  10 127.0.0.1 4010

ARGSNORMAL0 = 10 	127.0.0.1 8005  60000 127.0.0.1 8004  5000  127.0.0.1 8001 
ARGSNORMAL1 = 5000  127.0.0.1 8001  10 	  127.0.0.1 8005  10000 127.0.0.1 8003 
ARGSNORMAL2 = 10000 127.0.0.1 8003  5000  127.0.0.1 8001  15000 127.0.0.1 8000 
ARGSNORMAL3 = 15000 127.0.0.1 8000  10000 127.0.0.1 8003  20000 127.0.0.1 8002 
ARGSNORMAL4 = 20000 127.0.0.1 8002  15000 127.0.0.1 8000  25000 127.0.0.1 8009 
ARGSNORMAL5 = 25000 127.0.0.1 8009  20000 127.0.0.1 8002  30000 127.0.0.1 8006 
ARGSNORMAL6 = 30000 127.0.0.1 8006  25000 127.0.0.1 8009  35000 127.0.0.1 8007 
ARGSNORMAL7 = 35000 127.0.0.1 8007  30000 127.0.0.1 8006  50000 127.0.0.1 8008 
ARGSNORMAL8 = 50000 127.0.0.1 8008  35000 127.0.0.1 8007  60000 127.0.0.1 8004 
ARGSNORMAL9 = 60000 127.0.0.1 8004  50000 127.0.0.1 8008  10	127.0.0.1 8005 

# ARGS1 = 10 127.0.0.1 4010 130 127.0.0.1 4130  70 127.0.0.1 4070
# ARGS2 = 70 127.0.0.1 4070 10  127.0.0.1 4010  130 127.0.0.1 4130
# ARGS3 = 130 127.0.0.1 4130 70 127.0.0.1 4070  10 127.0.0.1 4010
# ARGS1 = 10 localhost 4010 130 localhost 4130  70 localhost 4070
# ARGS2 = 70 localhost 4070 10  localhost 4010  130 localhost 4130
# ARGS3 = 130 localhost 4130 70 localhost 4070  10 localhost 4010
# ARGS4 = 190 127.0.0.1 4190 130 127.0.0.1 4130  10 127.0.0.1 4010


ARGSSET = localhost 8000 SET TKN60 abc123 
ARGSGET =  localhost 4070 GET TKN70
ARGSDELETE= localhost 4070 DELETE TKN70


# ARGSALONE= 127.0.0.1 4070 70 
# ARGSJOIN1 = 127.0.0.1 4010 10 127.0.0.1 4070 70  
# ARGSJOIN2 = 127.0.0.1 4130 130 127.0.0.1 4010 10 
# ARGSJOIN3 =  127.0.0.1 4190 190 127.0.0.1 4130 130
# ARGSALONE= 127.0.0.1 8000 10 
# ARGSJOIN1 = 127.0.0.1 8001 5000 127.0.0.1 8000 10  
# ARGSJOIN2 = 127.0.0.1 8002 10000 127.0.0.1 8001 5000 
# ARGSJOIN3 =  127.0.0.1 8003 15000 127.0.0.1 8002 10000  
# ARGSJOIN4 =  127.0.0.1 8004 20000 127.0.0.1 8003 15000 
# ARGSJOIN5 =  127.0.0.1 8005 25000 127.0.0.1 8004 20000 
# ARGSJOIN6 =  127.0.0.1 8006 30000 127.0.0.1 8005 25000 
# ARGSJOIN7 =  127.0.0.1 8007 35000 127.0.0.1 8006 30000 
# ARGSJOIN8 =  127.0.0.1 8008 50000 127.0.0.1 8007 35000 
# ARGSJOIN9 =  127.0.0.1 8009 60000 127.0.0.1 8008 50000 

ARGSALONE=   127.0.0.1 8000 15000 
ARGSJOIN1 =  127.0.0.1 8001 5000  127.0.0.1 8000 15000 
ARGSJOIN2 =  127.0.0.1 8002 20000 127.0.0.1 8001 5000 
ARGSJOIN3 =  127.0.0.1 8003 10000 127.0.0.1 8002 20000  
ARGSJOIN4 =  127.0.0.1 8004 60000 127.0.0.1 8000 15000 
ARGSJOIN5 =  127.0.0.1 8005 10    127.0.0.1 8002 20000 
ARGSJOIN6 =  127.0.0.1 8006 30000 127.0.0.1 8001 5000 
ARGSJOIN7 =  127.0.0.1 8007 35000 127.0.0.1 8006 30000 
ARGSJOIN8 =  127.0.0.1 8008 50000 127.0.0.1 8005 10 
ARGSJOIN9 =  127.0.0.1 8009 25000 127.0.0.1 8005 10 



#########compiling###########,/
all:   per 
	# clie

per: peer.c	
	$(CC) $(CFLAGS) -o peer peer.c -lm hashtabelle.c

# clie: client.c	
# 	$(CC) $(CFLAGS) -o client client.c	

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



runnormal0: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL0}
runnormal1: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL1}
runnormal2: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL2}
runnormal3: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL3}
runnormal4: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL4}
runnormal5: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL5}
runnormal6: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL6}
runnormal7: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL7}
runnormal8: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL8}
runnormal9: peer 
	xterm -hold -e ./peer  ${ARGSNORMAL9}

runnormalall: runnormal0 runnormal1 runnormal2 runnormal3 runnormal4 runnormal5 runnormal6 runnormal7 runnormal8 runnormal9

rundelete: client 
	./client  ${ARGSDELETE}

runjoinall: runjoinall1 runjoinall2 runjoinall3

runjoinall1: runalone runjoin1 runjoin2 runjoin3
runjoinall2: runjoin4 runjoin5  runjoin6 runjoin7  
runjoinall3: runjoin8 runjoin9 

runalone:  peer 
	xterm -hold -e  ./peer  ${ARGSALONE}

runjoin1: peer 
	xterm -hold -e   ./peer  ${ARGSJOIN1}

runjoin2: peer 
	xterm -hold -e  ./peer  ${ARGSJOIN2}

runjoin3: peer 
	xterm -hold -e  ./peer  ${ARGSJOIN3}

runjoin4: peer 
	xterm -hold -e ./peer  ${ARGSJOIN4}

runjoin5: peer 
	xterm -hold -e ./peer  ${ARGSJOIN5}

runjoin6: peer 
	xterm -hold -e ./peer  ${ARGSJOIN6}

runjoin7: peer 
	xterm -hold -e ./peer  ${ARGSJOIN7}

runjoin8: peer 
	xterm -hold -e ./peer  ${ARGSJOIN8}

runjoin9: peer 
	xterm -hold -e ./peer  ${ARGSJOIN9}

runjoin10: peer 
	xterm -hold -e ./peer  ${ARGSJOIN10}

	
clean:
	rm peer
	rm client
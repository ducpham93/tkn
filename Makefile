
CC = gcc
CFLAGS  = -std=c99  -Wall 

ARGS = stratum2-4.NTP.TechFak.Uni-Bielefeld.DE ntp0.rrze.uni-erlangen.de time1.uni-paderborn.de




#########compiling###########,/
all:   clientNtp 

ntpClient: clientNtp.c	
	$(CC) $(CFLAGS) -o clientNtp  clientNtp.c  -lm 


#########run###########


run: clientNtp 
	./clientNtp  ${ARGS}

clean:
	rm clientNtp

	




	
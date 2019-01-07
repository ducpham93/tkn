/*
** talker.c -- a datagram "client" demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#define SERVERPORT "123" // the port users will be connecting to
#define MAXBUFLEN 48
#define UNIXOFFSET 2208988800

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

unsigned short char2short(unsigned char pchar[2])
{
	unsigned short pshort;
	pshort = (((short)pchar[0]) << 8) | pchar[1];
	return pshort;
}

double calculate_dispersion(double alldelays[8]){
	double mindelay = alldelays[0];
	double maxdelay = alldelays[0];
	for (int m = 1; m < 8; m++){
		if (alldelays[m] > maxdelay){
			maxdelay = alldelays[m];
		}
		if (alldelays[m] < mindelay){
			mindelay = alldelays[m];
		}	
	}	
	return maxdelay-mindelay;
}

unsigned char bytes[4];
unsigned short rootdis[2];

void int2short(unsigned int n){
	bytes[0] = (n >> 24) & 0xFF;
	bytes[1] = (n >> 16) & 0xFF;
	bytes[2] = (n >> 8) & 0xFF;
	bytes[3] = n & 0xFF;
	unsigned char tmp1[2];
	unsigned char tmp2[2];
	tmp1[0] = bytes[0];
	tmp1[1] = bytes[1];
	tmp2[0] = bytes[2];
	tmp2[1] = bytes[3];
	rootdis[0] = char2short(tmp1);
	rootdis[1] = char2short(tmp2);
}

long long int power(int x,int n)
{
    /* Variable used in loop counter */
    long long int number = 1;

    for (int j = 0; j < n; ++j)
        number *= x;

    return(number);
}

int digitnumber(unsigned int no){
	int totalDigits = 0;
	while(no){
		no = no/10;
		totalDigits++;
	}
	return totalDigits;
}

void printtime(unsigned int timetoprint[2]){
	time_t timet;
	timet = (const time_t) timetoprint[0];
	printf("%s", ctime(&timet));
	printf("Fractional digits:\t %u\n", timetoprint[1]);
}	

void printthetime(double time){
	time_t timet;
	timet = (const time_t) (unsigned int) time;
	unsigned int tmp = (unsigned int) time;
	printf("%s", ctime(&timet));
	printf("Fractional digits:\t %f\n", time-(double)tmp);
}	

double getowntime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	unsigned long int time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
	double mytime = (double)time_in_micros/1000000;
	return mytime;
}	

double calculate_offset(double start, double finish, unsigned int timeone[2], unsigned int timetwo[2]){
	double firstservertime = (double)timeone[0] + (double)((double)timeone[1]/(double)power(10,digitnumber(timeone[1])));
	double secondservertime = (double)timetwo[0] + (double)((double)timetwo[1]/(double)power(10,digitnumber(timetwo[1])));
	/*printf("Debugging: Digitnumber first: %d\n", digitnumber(timeone[1]));
	printf("Debugging: Digitnumber second: %d\n", digitnumber(timetwo[1]));
	printf("Debugging: first fractionnumber: %u\n", timeone[1]);
	printf("Debugging: second fractionnumber: %u\n", timetwo[1]);
	printf("Debugging: first todivideby: %lld\n", power(10,digitnumber(timeone[1])));
	printf("Debugging: second todivideby: %lld\n", power(10,digitnumber(timetwo[1])));
	printf("Debugging: first fraction: %f\n", (double)((double)timeone[1]/(double)power(10,digitnumber(timeone[1]))));
	printf("Debugging: second fraction: %f\n", (double)((double)timetwo[1]/(double)power(10,digitnumber(timetwo[1]))));
	printf("Debugging: Receive Timestamp: %f\n", firstservertime);
	printf("Debugging: Transmit Timestamp: %f\n", secondservertime);*/
	return 0.5*(firstservertime-start+secondservertime-finish);
}	

double calculate_delay(double start, double finish, unsigned int timeone[2], unsigned int timetwo[2]){
	double firstservertime = (double)timeone[0] + (double)((double)timeone[1]/(double)power(10,digitnumber(timeone[1])));
	double secondservertime = (double)timetwo[0] + (double)((double)timetwo[1]/(double)power(10,digitnumber(timetwo[1])));
	return firstservertime-start+finish-secondservertime;
}	

int main(int argc, char *argv[])
{
	printf("\n");
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	int rv;
	int numbytes;
	unsigned int buf[MAXBUFLEN];
	socklen_t addr_len;
	//char s[INET6_ADDRSTRLEN];
	unsigned int timerequest[12];
	//unsigned int reftime[2];
	//unsigned int ortime[2];
	unsigned int recvtime[2];
	unsigned int transtime[2];
	double rootdispersion;
	double avgoffset[argc-1];
	double avgdelay[argc-1];
	double avgrootdispersion[argc-1];
	double dispersion[argc-1];
	double alldelays[8];
	double dispersionsum[argc-1];
	
	for (int i = 0; i < 12; i++){
		timerequest[i] = 0;
	}	
	
	if (argc == 1) {
		fprintf(stderr,"usage: ntpclient server1 server2 server3...\n");
		exit(1);
	}
	
	for (int k = 0; k < argc-1; k++){
		avgoffset[k] = 0;
		dispersion[k] = 0;
		avgdelay[k] = 0;
		avgrootdispersion[k] = 0;
	}	
	
	timerequest[0] = 35; // version = 4 and mode = 3 (i have no idea why that works)
	
	for (int j = 1; j < argc; j++){			// loop for each server
	
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		if ((rv = getaddrinfo(argv[j], SERVERPORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}
		// loop through all the results and make a socket
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
				perror("talker: socket");
			continue;
			}
			break;
		}
		if (p == NULL) {
			fprintf(stderr, "talker: failed to create socket\n");
			return 2;
		}
	
		freeaddrinfo(servinfo);
		for (int i = 0; i < 8; i++){		// loop 8 times for each server
			
			double start = getowntime();
			if ((numbytes = sendto(sockfd, timerequest, 48, 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("ntpclient: sendto");
				exit(1);
			}
			//printf("ntpclient: sent %d bytes to %s\n", numbytes, argv[1]);
			
			//printf("listener: waiting to recvfrom...\n");
			addr_len = sizeof(their_addr);
			if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
			double finish = getowntime();
			buf[numbytes] = '\0';
			
			//reftime[0] = ntohl(buf[4]) - UNIXOFFSET;
			//reftime[1] = ntohl(buf[5]) - UNIXOFFSET;
			//ortime[0] = ntohl(buf[6]) - UNIXOFFSET;
			//ortime[1] = ntohl(buf[7]) - UNIXOFFSET;
			recvtime[0] = ntohl(buf[8]) - UNIXOFFSET;
			recvtime[1] = ntohl(buf[9]) - UNIXOFFSET;
			transtime[0] = ntohl(buf[10]) - UNIXOFFSET;
			transtime[1] = ntohl(buf[11]) - UNIXOFFSET;
			
			int2short(ntohl(buf[2]));
			rootdispersion = (double)rootdis[0] + (double)((double)rootdis[1]/(double)power(10,digitnumber((unsigned int)rootdis[1])));
			
			/*printf("Reference Timestamp:\t %u.%u\n", reftime[0], reftime[1]);
			//printf("Origin Timestamp:\t %u.%u\n", ortime[0], ortime[1]);
			printf("Starting Timestamp:\t %f\n", start);
			printf("Receive Timestamp:\t %u.%u\n", recvtime[0], recvtime[1]);
			printf("Transmit Timestamp:\t %u.%u\n", transtime[0], transtime[1]);
			printf("Finishing Timestamp:\t %f\n", finish);
			//printf("Root Dispersion:\t %u.%u\n", rootdis[0], rootdis[1]);
			printf("Root Dispersion:\t %f\n", rootdispersion);*/
			
			double offset = calculate_offset(start, finish, recvtime, transtime);
			double delay = calculate_delay(start, finish, recvtime, transtime);
			
			avgoffset[j-1] += offset;
			avgdelay[j-1] += delay;
			avgrootdispersion[j-1] += rootdispersion;
			
			alldelays[i] = delay;
			
			/*printf("Offset:\t\t %f\n", offset);
			printf("Delay:\t\t %f\n", delay);*/
		
			//printtime(recvtime);
			//getowntime();
			
		}	
		avgoffset[j-1] /= 8;
		avgdelay[j-1] /= 8;
		avgrootdispersion[j-1] /= 8;
		dispersion[j-1] = calculate_dispersion(alldelays);
		dispersionsum[j-1] = dispersion[j-1] + avgrootdispersion[j-1];
		printf("Statistics for server %s:\n", argv[j]);
		printf("Average Root Dispersion:\t %f\n", avgrootdispersion[j-1]);
		printf("Average Dispersion:\t\t %f\n", dispersion[j-1]);
		printf("Average Delay:\t\t\t %f\n", avgdelay[j-1]);
		printf("Average Offset:\t\t\t %f\n", avgoffset[j-1]);
		printf("\n");
	}
	double selectserver = dispersionsum[0];
	int minindex = 0;
	for (int b = 1; b < argc-1; b++){
		if (dispersionsum[b] < selectserver){
			selectserver = dispersionsum[b];
			minindex = b;
		}
	}	
	
	
	printf("__________________________________________________________________\n");
	printf("Server %s has won the race!\n", argv[minindex+1]);
	printf("System time:\n");
	printthetime(getowntime());
	printf("Average Offset: %f\n", avgoffset[minindex]);
	printf("Updated system time:\n");
	printthetime(getowntime()+avgoffset[minindex]);
	printf("\n");
	
	close(sockfd);
	return 0;
}
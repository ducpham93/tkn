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
#define MAXDATASIZE 10000
//#define SERVERPORT "4950" // the port users will be connecting to

#define BILLION  1E9;

// Calculate time taken by a request
struct timespec requestStart, requestEnd;

int main(int argc, char *argv[])
{
	clock_gettime(CLOCK_REALTIME, &requestStart);
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char buf[MAXDATASIZE];
	int numbytes;
	int receive;
	if (argc != 4) {
		fprintf(stderr,"usage: clientudp hostname message port\n");
		exit(1);
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if ((rv = getaddrinfo(argv[1], argv[3], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("clientudp: socket");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "clientudp: failed to create socket\n");
		return 2;
	}
	if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
		p->ai_addr, p->ai_addrlen)) == -1) {
		perror("clientudp: sendto");
		exit(1);
	}
	freeaddrinfo(servinfo);
	
	if ((receive = recvfrom(sockfd, buf, MAXDATASIZE-1, 0,
		p->ai_addr, &p->ai_addrlen)) == -1) {
		perror("clientudp: receive from");
		exit(1);
	}
	buf[receive] = '\0';
	printf("clientudp: sent %d bytes to %s\n", numbytes, argv[1]);
	printf("%s\n",buf);
	close(sockfd);
	
	clock_gettime(CLOCK_REALTIME, &requestEnd);

	// Calculate time it took
	double accum = ( requestEnd.tv_sec - requestStart.tv_sec )
	+ ( requestEnd.tv_nsec - requestStart.tv_nsec )
	/ BILLION;
	printf( "Time measured: %lf s\n", accum);
	
	return 0;
}
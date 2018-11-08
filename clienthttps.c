#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
//#define PORT "17" // the port client will be connecting to 
#define MAXDATASIZE 200000 // max size of image file in bytes
// get sockaddr, IPv4 or IPv6:
#include<fcntl.h> //fcntl
 
//Size of each chunk of data received, try changing this
#define CHUNK_SIZE 1


#define BILLION  1E9;

char buf[MAXDATASIZE];

int recv_timeout(int s , int timeout)
{
    int size_recv , total_size= 0;
    struct timeval begin , now;
    char chunk[CHUNK_SIZE];
    double timediff;
	int curpos = 0;
	
	/*for (int i = 0; i < MAXDATASIZE; i++){
		buf[i] = 0;
	}*/	
     
    //make socket non blocking
    fcntl(s, F_SETFL, O_NONBLOCK);
     
    //beginning time
    gettimeofday(&begin , NULL);
     
    while(1)
    {
        gettimeofday(&now , NULL);
         
        //time elapsed in seconds
        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
         
        //if you got some data, then break after timeout
        if( total_size > 0 && timediff > timeout )
        {
            break;
        }
         
        //if you got no data at all, wait a little longer, twice the timeout
        else if( timediff > timeout*2)
        {
            break;
        }
         
        memset(chunk ,0 , CHUNK_SIZE);  //clear the variable
        if((size_recv =  recv(s , chunk , CHUNK_SIZE , 0) ) < 0)
        {
            //if nothing was received then we want to wait a little before trying again, 0.1 seconds
            usleep(100000);
        }
        else
        {
            total_size += size_recv;
			
			if (strlen(chunk)){
				for (int i = curpos; i < (curpos + strlen(chunk)); i++){
					buf[i] = chunk[i-curpos];
				}
				curpos += strlen(chunk);
			}
			else {
				buf[curpos] = 00;
				curpos++;
			}	
            gettimeofday(&begin , NULL);
        }
    }
     
    return total_size;
}

char *p;

int strpos(char *hay, char *needle, int offset)
{
   char haystack[strlen(hay)];
   strncpy(haystack, hay+offset, strlen(hay)-offset);
   char *p = strstr(haystack, needle);
   if (p)
      return p - haystack+offset;
   return -1;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[])
{
	
	if (argc != 2) {
        fprintf(stderr,"usage: client url\n");
        exit(1);
    }
	
    int sockfd, numbytes;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
	
	const char* lineConst = argv[1]; // the url
	char line[100];  // where we will put a copy of the input
	char *hostname; // the "result"
	char *path;
	
	/*for (int i = 0; i < 100; i++){
		line[i] = 0;
	}*/
	
	strncpy(line, lineConst, strlen(lineConst));
	
	hostname = strtok(line,"//"); // find the first double slash
	hostname = strtok(NULL,"/");   // find the third slash
	path = strtok(NULL, "'\0'");  // its the path from here
	
	//printf("hostname: %s\n", hostname);
	
	//printf("path: %s\n", path);
	
	if (path == NULL){
		fprintf(stderr,"no path\n");
        exit(1);
	}
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(hostname, "80", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    //printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
	
	char* get = "GET /";
	char* host = "HOST: ";
	char* misc = "\r\n";
	char* https = " HTTP/1.1\r\n";
	
	char *buffer = malloc (sizeof(char)* (strlen(get) + strlen(path) + strlen(https) + 
						   strlen(host) + strlen(hostname) + 
						   strlen(misc) + strlen(misc) + 1));
	if (buffer == NULL) {
		// Out of memory.
	} 
	else {
		strcpy (buffer, get);
		strcat (buffer, path);
		strcat (buffer, https);
		strcat (buffer, host);
		strcat (buffer, hostname);
		strcat (buffer, misc);
		strcat (buffer, misc);
		send(sockfd, buffer, strlen(buffer), 0);
		free (buffer);
	}
	
	int totsize = recv_timeout(sockfd, 3);
	
	int pos = strpos(buf, "\r\n\r\n", 0);
	pos += 4;
	
	for (int i = pos; i < totsize; i++){
		printf("%c",buf[i]);
	}	
    close(sockfd);

    return 0;
}
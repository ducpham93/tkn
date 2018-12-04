#define _POSIX_C_SOURCE 199309L  // for clock get time to work 
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <stdarg.h>
#include <limits.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/socket.h>		
#include <netinet/in.h>		
#include <poll.h>
#include <netdb.h>		
#include <arpa/inet.h>		
#include <arpa/telnet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h> 
#include <string.h> 
#include <ctype.h>
#include <time.h>


int main(int argc, char** args)
{
	struct timespec start ,end;
	clockid_t id;
	
	id = CLOCK_MONOTONIC ;

	clock_gettime( id, &start) ;
	if (argc < 2 )
	{
		perror("error please type Dns or ip and a port number \n");
		exit(1) ;
	}
	else if (args[1]== NULL)
	{
		perror("error please type Dns or ip and  \n");

	}else if (args[2]== NULL)
	{
		perror("error please type port ") ;
	}


/*
	****************parsing*****************
*/
	char ip_Dns[100] = "";        //DNS
	strcpy(ip_Dns,args[1]); // copy Dns
	
/*
	****************sockets_create*****************
*/

	
	int CS ; // client socket

	CS = socket(PF_INET,SOCK_STREAM,0);  // creating client socket 
	if(CS<0)
		perror("cant make client socket") ;
/*
	****************sockets_intialising*****************
*/
	struct sockaddr_in SA ;

	bzero(&SA,sizeof(SA)); // putting zeros in it 

 	SA.sin_port = htons(atoi(args[2])) ;
	SA.sin_family = AF_INET ;
	if( isdigit(args[1][0]))
	{
		
			SA.sin_addr.s_addr = inet_addr(ip_Dns) ;
	}
	else
	{	

        struct hostent* info=gethostbyname(ip_Dns);// turn domain to hexi 
        struct in_addr **listi;
        listi = (struct in_addr **) info->h_addr_list; //
 
        // printf("%s\n",ip_Dns );
         SA.sin_addr.s_addr = inet_addr(inet_ntoa(*listi[0]));  // turn hexi to ip 

	
	}// turn address to to binary
		
	 
/*
	****************connecting*****************
*/
	if(connect(CS,(struct sockaddr *) &SA,sizeof(SA))<0)
		 perror("cant connect client ") ;// socket, address of host , addres length

	char  text[10000]  ;
	recv(CS,text,10000,0) ;
	printf("%s",text );
	// 


/*
	****************closing*****************
*/
	close(CS);
	clock_gettime( id, &end);

	
	 double s = ((double)end.tv_sec + (double)end.tv_nsec / 1000000000.0)-(((double)start.tv_sec + (double)start.tv_nsec / 1000000000.0)) ;
	printf("%lf\n",s );
	
	// printf("%s,%s",args[1],args[2]);


}
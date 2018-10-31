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



int main(int argc, char** args)
{
	// printf("safe\n");
	if (argc < 1 )
	{
		perror(" please url \n");
		exit(1) ;
	}
	else if (args[1] == NULL)
	{
		perror("error please type Dns or ip and  \n");

	}

	int flag = 0 ;
	// printf("safe %d\n",flag);

	// printf("%s\n", args[3]);


	  if(   args[1][strlen(args[1]) - 3] == 'j'
	        && args[1][strlen(args[1]) - 2] == 'p'
	        && args[1][strlen(args[1]) - 1] == 'g'
	    )
		{
			flag = 1  ;
			// if (!strcmp( args[2], "0"))   // set flag if u have to  print the picture out
			// {
			// 	;
			// 	// printf("safe %d\n", flag);
			// }
			// else
			// 	printf("pleas write  in the right form\n");

		}

		// printf("%d\n",flag );


	/*
		****************parsing*****************
	*/
	char ip_Dns[100] = "";        //DNS
	char pfad[100] = "";        //DNS
	int i = 0 ;
	int count = 0;
	int ipcounter = 0 ;
	int pfadcounter = 0 ;
	// printf("%d    \n", strlen(args[1]));

	while (i < strlen(args[1]))
	{

		if ((args[1][i]) == '/')
			count++;


		if (count == 2)
		{
			ip_Dns[ipcounter] = args[1][i + 1];
			ipcounter++;
		}
		else if (count == 3)
		{
			pfad[pfadcounter] = args[1][i];
			pfadcounter++;
		}
		i++;
	}
	pfad[pfadcounter] = '\0';
	ip_Dns[ipcounter - 1] = '\0';


	char request [100 ] ;                     // parsing th request
	sprintf(request, "GET %s HTTP/1.1\r\nHOST: %s \r\n\r\n", pfad, ip_Dns) ;
	 // printf("%s\n", request );


	/*
		****************sockets_create*****************
	*/



	int CS ; // client socket

	CS = socket(PF_INET, SOCK_STREAM, 0); // creating client socket
	if (CS < 0)
		perror("cant make client socket") ;

	/*
		****************sockets_intialising*****************
	*/
	struct sockaddr_in SA ;

	bzero(&SA, sizeof(SA)); // putting zeros in it

	SA.sin_port = htons(80) ; // tcp port
	SA.sin_family = AF_INET ;
	if ( isdigit(args[1][0])) // if ip als eingabe
	{

		SA.sin_addr.s_addr = inet_addr(ip_Dns) ;
	}
	else      // if domain als eingabe
	{

		struct hostent* info = gethostbyname(ip_Dns); // turn domain to hexi
		struct in_addr **listi;

		listi = (struct in_addr **) info->h_addr_list; //



		SA.sin_addr.s_addr = inet_addr(inet_ntoa(*listi[0]));  // turn hexi to ip


	}// turn address to to binary


	/*
		****************connecting*****************
	*/
	if (connect(CS, (struct sockaddr *) &SA, sizeof(SA)) < 0)
		perror("cant connect client ") ;// socket, address of host , addres length

	char  text[100000]  = { 0 }; ;
	
	send(CS, request, strlen(request), 0) ;
	int byts = recv(CS, text, 100000, 0) ; // byts == how many bytes recieved

	/*********************jpg + Header processing********************/
	// printf("%d\n",flag );
	if (flag == 1)
	{

	/*******************parsing Header *****************************/
		char * res = strstr(text, "\r\n\r\n"); // return the the subsrting
		int j = abs((int) res)  - abs((int) text) ;// calc ulating the index
		j = abs(j) +4   ; // index of data
		
		char * tmp  = strstr(text, "Content-Length: "); // return the the subsrting
		int x = abs((int) tmp)  - abs((int) text) ;// calc ulating the indexof the picture length
		x = abs(x) + 16 ; // first index of data of length
		int y = x ;
		int move = 0 ;

		char  lenContext[20] ;
		while (text[y] != '\r')  // moving sub string
		{
			lenContext[move] = text[y] ;
			move ++;
			y++;

		}
		move = 0 ;
		int multi = atoi(lenContext) ;          // turning  string to integer

    /****************************writing to file *******************/

		FILE* Q = fopen("immmmage.jpg", "wb");  // writing in binary mode
		// FILE* fp = fopen("Et2gFH3", "rb");  // writing in binary mode

	
   		

   		 // char buf[100000]  ;
   		 size_t i = j;
   		 x = 0 ;

   		 for (; i < byts; ++i) // copying data without header
   		 {
   		 	 // fputc(text[i],Q);
   		 	 
   		 	 

   		 	 x++;
   		 }
   		 res += 4 ;
   		 // printf("%d\n",x);
   		 fwrite(res, 1, byts - j, stdout) ;

	    
	
		 
	
		// char  textNew[100000]  = { 0 } ;
		// char  modified[100000] = { 0 }  ;
		
		// printf("%d %d  %s \n", strlen(textNew), multi , text);
		byts -= j ;
		while ( byts < multi ) // recive all data and copy it into text new - header
		{
			int reccc = recv(CS, text, 100000, 0) ;
			byts += reccc ;

			fwrite(text, 1, reccc, stdout) ;
			bzero(text,100000);
			// strcat(textNew, text);
			// fwrite(text, sizeof(char), reccc , Q);

		 	
		}
		// printf("bytes %d    mult : %d\n",byts , multi);
		
		fclose(Q);

			// printf("%c", text[i] );
			// unsigned char c 	=(unsigned char)text[i];
		


		
		
	}




	/*
		****************closing*****************
	*/
	close(CS);
	

	// printf("%s,%s",args[1],args[2]);


}
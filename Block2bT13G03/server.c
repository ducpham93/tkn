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
#include <time.h> 


int main(int argc, char** args)
{
	


/*
	****************parsing file*****************
*/
	

	if (argc < 2 )
	{
		printf("error please type filename and a port number \n");
		exit(1);

	}
	else if (args[2]== NULL)
	{
		printf("plz insert file location");
		exit(1);

	}else if (args[1]== NULL)
	{
		printf("error please type port ") ;
		exit(1);
	}

	FILE *Q ;
	int leni = 1;

	Q = fopen(args[2],"r") ;

	if (Q == NULL)
	{
		printf("not opene file ");
		exit(1);
	}


      char c = getc(Q) ;
	while(c!= EOF) // count lines 
	{
		if (c=='\n')	
			leni++;
		c = getc(Q) ;
	}

/*
	****************sockets_create(for local host)*****************
*/

	int LS ; // listen socket

	LS = socket(PF_INET,SOCK_STREAM,0);  // creating listen socket 
	if(LS<0)
	{
		printf("cant make server socket") ;
		exit(1);
	}



/*
	****************sockets_intialising(for local host)*****************
*/
	struct sockaddr_in SA ;

	bzero(&SA,sizeof(SA)); // putting zeros in it 

 	SA.sin_port = htons(atoi(args[1])) ;
	SA.sin_family = AF_INET ;
	SA.sin_addr.s_addr = htonl(INADDR_ANY) ; //   local host or INADDR_ANY ***** INet oderr htons  nocmal fragen *****

	if(bind(LS,(struct sockaddr *) &SA,sizeof(SA))<0) // socket, address of host , address length
	{
		 printf("cant connect Server bind ") ;
		 exit(1);
	}

/*
	****************Connection_Qeue_creating*****************
*/
	if(listen(LS,18) == -1 ) // 18 = how many
	{ 
		printf("cant listen Server  ") ;
		exit(1);
	}


/*
	****************sockets_intialising(for client address)*****************
*/
	struct sockaddr_in CA ;

/*
	****************Connection_accepting and receiving *****************
*/		
	int CS  ;

	
	
	char Buffer [512]  ;
		srand(time(0));// getting rand to function with time
		
		
		

		socklen_t  s = sizeof(CA) ;
		CS = accept(LS,(struct sockaddr *) &CA,&s);
		if( CS < 0 ) // 1accepting
		{
			printf("cant  accept  connection Server  ") ;
			exit(1);
	    }

		Q = fopen(args[2],"r") ;
		if(!Q)
			printf("cant open file \n");
		int n = rand();      // getting random number 
		n = n % leni ;
		int i = 0 ;
		n= 0 ;
		
		while(fgets(Buffer, 512,Q))  // readinf file and checking if right line 
		{
			if (n== i)
				break ;
			i++ ;
		}
		
		// 

		// printf("%d\n",strlen(Buffer) );
		send(CS,Buffer,strlen(Buffer),0) ;                // send quote of the day 
		
	

/*
	****************recieving*****************
*/	
	
	




/*
	****************closing*****************
*/
	close(CS);
	close(LS);
	fclose(Q); // closing file 

}



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
#include <sys/select.h>



int main(int argc, char** args)
{
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

	char * befehel  = args[3] ;
	char * key = args[4] ;
	char * data = args[5] ;


/*
	****************parsing*****************
*/
	char ip_Dns[100] = "";        //DNS
	strcpy(ip_Dns,args[1]); // copy Dns
	
/*
	****************sockets_create*****************
*/

	
	int CS ; // client socket

	CS = socket(PF_INET,SOCK_STREAM | SOCK_NONBLOCK,0);  // creating client socket 
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
     
       
  
       SA.sin_addr.s_addr = inet_addr(inet_ntoa(*listi[0]));  // turn hexi to ip 


	}// turn address to to binary
		
	 
/*
	****************connecting*****************
*/
	if(connect(CS,(struct sockaddr *) &SA,sizeof(SA))<0)
		 perror("cant connect client ") ;// socket, address of host , addres length
    
	char  text[10000]  = {0};

    bzero(text,10000);
    text[0]= 0 ;

    if (!strcmp(befehel,"GET"))
    {
    	text[0] =  4 ;
    }
    else if (!strcmp(befehel,"SET"))
    {
    	text[0] =  2 ;
    }
    else if (!strcmp(befehel,"DELETE"))
    {
    	text[0] =  1 ;
    }
    

    int lenkey = (int)(strlen(key))  ;
    int lendata= 0  ;
    if (data)
   	 lendata = (int)(strlen(data))  ;
   	else
   		lendata = 0 ;

   srand(time(0));// getting rand to function with time
    text[1]=  rand() % 128 ;        // need to generate id  (128 biggest asci value )
    text[2]= (char)lenkey>>8 ;
    text[3]= (char)lenkey ;
    text[4]= (char)lendata>>8   ;
    text[5]=  (char) lendata ;

    	printf("%d %d\n",  text[2] , text[4] );




  
    
    strncpy(&text[6],key,lenkey);
    int textsize =  6 +lenkey;

    if (!strcmp(befehel,"SET"))
    {
    	 strncpy(&text[textsize],data,lendata);
    	 textsize += lendata ;
    }


    
   
   
	fwrite(text,1,textsize,stdout) ;

   

   // time_t timer = 2;  // in seconds 
   // time_t start = time(NULL);
   // time_t end = start + timer;
	int timer = 2; /* 10ms */
	clock_t start = clock();
    
   while(1)
   {

	   int bytesent = send(CS,text,textsize,0);

	   if(bytesent !=textsize)
	     perror("error while sending request")   ;
		 for (int i = 0; i < bytesent; ++i)
	    {
	    	printf("request is ,   i ist  %d and  %d  \n",i,text[i]);
	    }
			

	     sleep(0.2); // give some time to recive 

	    // bzero(text,10000);

	    int bytes =0;
	    bytes=  recv(CS,text,10000,0) ;


	    if(!bytes)
	     printf("error while reciving aknowldgment\n")   ;
	  	for (int i = 0; i < bytes; ++i)
	    {
	    	printf("answer is ,   i ist  %d and  %d  \n",i,text[i]);
	    }

 		 clock_t d = clock() - start;
 		 d = d / CLOCKS_PER_SEC;
         
	    if (bytes > 0||  d >= timer )  // break timer 
	    {
	    	break ;
	    }
	 }



/*
	****************closing*****************
*/	
	close(CS);
	
	// printf("%s,%s",args[1],args[2]);


}
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
#include "hashtabelle.h"


#define HASHSIZE 10


int main(int argc, char** args)
{
	

if (args[1]== NULL)
	{
		printf("error please type port ") ;
		exit(1);
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

	
	
	char Buffer [1000000]  ;
	




	while(1)	
	{
		

		socklen_t  s = sizeof(CA) ;
		CS = accept(LS,(struct sockaddr *) &CA,&s);
		if( CS < 0 ) // accepting
		{
			printf("cant  accept  connection Server  ") ;
			exit(1);
	    }


        DataItem_i* *hashArray =erstelle(HASHSIZE);

		int bytes = recv(CS,Buffer,1000000,0) ; 

		int x = 1 ;
		int  index4 = 0 ;
		int  index5 = 0 ;
		int  index6 = 0 ;
		int  index7 = 0 ;
		
		index4 = Buffer[0]  & (x << 3 ) ;     // get first bite if set to one index4 will be 8
    	index5 = Buffer[0]  & (x << 2 ) ;  // get 2 bite if set to one index4 will be 4
		index6 = Buffer[0]  & (x << 1 ) ;  // get 2 bite if set to one index4 will be 2
		index7 = Buffer[0]  & x   ;		// get 1 bite if set to one index4 will be 2
		
		if ((index4+index5+index6+index7)!= 1)
		{
			perror("smth went wrong with request Header ");
		}
		char tid = Buffer[1];

	   
        uint16_t   keylen= Buffer[3] + (Buffer[2] <<8 );         // copying the key  len
        uint16_t   datalen = Buffer[5] + (Buffer[4] <<8 );       // copying the data  len

        bzero(Buffer,1000000) ;
        int i = bytes ;
		while(bytes>0)
		{
			 bytes = recv(CS,&Buffer[i],1000000,0) ;               
			 i += bytes;			 
		}


        char* key= Buffer +48  ;
        char* data = key + keylen  ;   
       

        char answer [1000000]  ;
        bzero(answer,1000000) ;
        answer[0]= 8;  // set akn
       	answer[1]= tid;  // set id
       	int answersize = 48 ;  // only
        

        if (index5)
        {
        	DataItem_i *requested = get(hashArray,HASHSIZE,key , keylen  );
        	if( !requested )
        	     perror("(get )key not found ") 	;
        	 else{

	        	 answer[0]= 12;  // set get and akn
	        	 
	        	 answer[2]=  requested->keylen   & 0xFF ;// set last 8 bits to 0
	        	 answer[3]= (requested->keylen >> 8  & 0xFF);          	 
	        	 answer[4]=  requested->datalen  & 0xFF;   //set last 8 bits to 0
	        	 answer[5]= (requested->datalen >> 8)  & 0xFF;  
	        	 memcpy( &answer[6], requested->key, requested->keylen );
	        	 memcpy( &answer[6+requested->keylen ], requested->data, requested->datalen );
	        	 answersize+=  requested->keylen ;
	        	 answersize+=   requested->datalen ;
	        	}


        } 
        else if (index6)
        {
        	
        	if(set(hashArray,HASHSIZE,key,data , keylen ,datalen ) == -1)
        	     perror(" set hashtabele is full ") 	;
        	 answer[0]= 10;  // set set and akn

        }      
        else if (index7)
        {
        	if(delete(hashArray,HASHSIZE,key, keylen  ) == -1)
        	     perror("(delete )could not find the item to delete") 	;
        	 answer[0]= 9;  // set delete and akn

        }
       


       int bytesent = send(CS,answer,answersize,0);

       if(bytesent )




   	    

 		



		


/*
***************************start to parse********************

*/
		close(CS);

	}		
	

/*
	****************recieving*****************
*/	
	
	




/*
	****************closing*****************
*/
	
	// close(LS);


}



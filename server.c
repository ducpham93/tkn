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


#define HASHSIZE 3


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

  
  
  char Buffer [10000]  ;
  


  DataItem_i* *hashArray =erstelle(HASHSIZE);

  while(1)  
  {
    

    socklen_t  s = sizeof(CA) ;
    CS = accept(LS,(struct sockaddr *) &CA,&s);
    if( CS < 0 ) // accepting
    {
      printf("cant  accept  connection Server  ") ;
      exit(1);
      }
    

   
     bzero(Buffer,10000) ;
    recv(CS,Buffer,10000,0) ; 
   
    
   
    int x = 1 ;
    
    int  index5 = 0 ;   // for get 
    int  index6 = 0 ;  // for set 
    int  index7 = 0 ;    // for delete 
    
   
    index5 = Buffer[0]  & (x << 2 ) ;  // get 1 bite if set to one index5 will be 4
    index6 = Buffer[0]  & (x << 1 ) ;  // get 1 bite if set to one index6 will be 2
    index7 = Buffer[0]  & x   ;   // get 1 bite if set to one index7 will be 1
  //  printf("%d\n",index7 );
    


     
    uint16_t   keylen= Buffer[3] + (Buffer[2] <<8 );         // copying the key  len
    uint16_t   datalen = Buffer[5] + (Buffer[4] <<8 );       // copying the data  len

    
    // int i = bytes ;
    // printf("sicher1\n");
     
    // while(bytes>0)
    // {
    //    bytes = recv(CS,&Buffer[i],10000,0) ;               
    //    i += bytes;         
    // }
    
    
    char tid = Buffer[1];
     
     

    
    char* key= NULL ;
    char* data= NULL ;
    key = Buffer +6  ;
     data = key + keylen  ; 
    char  keydebugguing[10000] ;
    char  datadebugguing[10000] ;
    strncpy(keydebugguing,&Buffer [6]  ,keylen); 
    keydebugguing[6+keylen] = '\0' ;
    memcpy(datadebugguing,&Buffer[6+keylen ]  ,datalen); 
    // datadebugguing[5+keylen+datalen] = '\0' ;
    
   

    char answer [10000]  ;
    bzero(answer,10000) ;
    answer[0]= 8;  // set akn

//TODO change back to tid 
    

    answer[1]=tid ;  // set id
    int answersize = 6 ;  // only
    
    
    if (index5)
    {


      DataItem_i *requested = get(hashArray,HASHSIZE,keydebugguing , keylen  );
       if( !requested )
       {
           perror("(get )key not found ")   ;
       }
       else
       {
         printf("retrived value succefully\n" );
         answer[0]= 12;  // set get and akn
         
         answer[2]=  requested->keylen   & 0xFF ;// set last 8 bits to 0
         answer[3]= (requested->keylen >> 8  & 0xFF);            
         answer[4]=  requested->datalen  & 0xFF;   //set last 8 bits to 0
         answer[5]= (requested->datalen >> 8)  & 0xFF;  

         strncpy( &answer[6], requested->key, requested->keylen );
         memcpy( &answer[6+requested->keylen ], requested->data, requested->datalen );
         answersize+=  requested->keylen ;
         answersize+=   requested->datalen ;
        }
        printf("%s  and  %d \n",keydebugguing,keylen );


    } 
    else if (index6)
    {

      printf("%s  and  %d \n",keydebugguing,keylen );
       
      int r = set(hashArray,HASHSIZE,keydebugguing,data , keylen ,datalen ) ;
      if(r == -1)
      {
           perror(" set hashtabele is full ")   ;
      }
      else if (r== 0)
      {
        printf("added succefully\n");
      }
      else if (r == 1)
      {
        printf("updated succefully\n");
      }
      else if (r == 2)
      {
        printf("already inside succefully\n");
      }
       answer[0]= 10;  // set set and akn

     

    }      
    else if (index7)
    {   
      //   for (int x = 0; x < i; ++x)
      // {
      //   printf("sicher1 ,   i ist  %d and  %c  \n",x,Buffer[x]);
      // }

      printf("%s  and  %d \n",keydebugguing,keylen );
      if(delete(hashArray,HASHSIZE,keydebugguing, keylen  ) == -1)
           perror("(delete )could not find the item to delete")   ;
      else
        printf("deleted succefully\n");
       answer[0]= 9;  // set delete and akn

    }
   

   // fwrite(answer,1,answersize,stdout) ;
   //      printf("%d\n",(int) strlen(answer));

    
// for (int x = 0; x < answersize; ++x)
//  {
//     printf("%d and %d \n",x, answer[x]);
//  } 

   int bytesent = send(CS,answer,answersize,0);

   if(bytesent !=answersize)
     perror("error while sending aknowldgment")   ;




    





    


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

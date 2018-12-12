/*
** selectserver.c -- a cheezy multiperson chat server
*/

#define _XOPEN_SOURCE 600 /// fix 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <stdarg.h>
#include <limits.h>
#include <setjmp.h>
#include <arpa/telnet.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <ctype.h>



#include "hashtabelle.h"

#define HASHSIZE 10
#define DHTSIZE 255
#define CLIENTSSIZE 10
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int hashingclient( char *key, uint16_t len, int SIZE, char *id_prev, char *id_self, char *id_next) // reutrn 1 if responible else 0
{
    int hash = 5381;
    key = key + 14;

    for (int i = 0; i < len; ++i)
    {
        hash = ((hash << 5) + hash) +  (*key); /* hash * 33 + c */
        key++ ;

    }
    hash = hash % SIZE ;
    printf("(hashs is  %d)\n",hash );
    if ((hash <= atoi(id_self) && hash > atoi(id_prev)) || ( atoi(id_prev) >atoi(id_self) && hash > atoi(id_prev) && hash > atoi(id_self))
            || (atoi(id_prev) >atoi(id_self) && hash >= 0 && hash < atoi(id_self)) )
    {
        return 1;
    }
    return  0;
}

void convert_client_protocol(char *oldBuffer, char *newprotococlient, char *id_self,
                             char *ip__self,  char *port_self)
{
    memcpy(newprotococlient, oldBuffer, 6); // copy first 6 bytes in the new protocol
    newprotococlient[0] = (newprotococlient[0] | 1 << 7) ;
    newprotococlient[6] = atoi(id_self) >> 8 ;
    newprotococlient[7] = atoi(id_self) ;

    unsigned long x;
    x = inet_addr(ip__self); // convert ip to long
    newprotococlient[8] = x >> 24 ;
    newprotococlient[9] = x >> 16 ;
    newprotococlient[10] = x >> 8 ;
    newprotococlient[11] = x ;
    newprotococlient[12] = atoi(port_self) >> 8 ;
    newprotococlient[13] = atoi(port_self) ;
    printf("convert_client_protocol %d\n", newprotococlient[0]);

    /*++++++++++++++++++++copying key and value ++++++++++++++++*/
    uint16_t   keylen = oldBuffer[3] + (oldBuffer[2] << 8 );       // copying the key  len
    uint16_t   datalen = oldBuffer[5] + (oldBuffer[4] << 8 );      // copying the data  len
    memcpy(&newprotococlient[14], &oldBuffer[6], keylen); // copy key 6
    memcpy(&newprotococlient[14 + keylen], &oldBuffer[6 + keylen], datalen); // copy data
}

int convert_client_protocol_back( char *newprotococlient, char *oldBuffer)
{
    memcpy(newprotococlient, oldBuffer, 6); // copy first 6 bytes in the new protocol
    // oldBuffer[0] = oldBuffer[0] <<
    newprotococlient[0] = newprotococlient[0] + 128 ;// get rid of the 1 for internal massages
    /*++++++++++++++++++++copying key and value ++++++++++++++++*/
    uint16_t   keylen = oldBuffer[3] + (oldBuffer[2] << 8 );       // copying the key  len
    uint16_t   datalen = oldBuffer[5] + (oldBuffer[4] << 8 );      // copying the data  len
    memcpy(&newprotococlient[6], &oldBuffer[14], keylen); // copy key 6
    memcpy(&newprotococlient[6 + keylen], &oldBuffer[14 + keylen], datalen); // copy data
    return (6 + keylen + datalen) ;
}
void send_to_next_node( char *request, int RequestSize, char *ip_next,  char *port_next, int flag_parse_portback)
{
    int CS ; // client socket
    CS = socket(PF_INET, SOCK_STREAM , 0); // creating client socket
    if (CS < 0)
        perror("cant make client socket") ;
    unsigned char * port_unsigned =   (unsigned char*) port_next ; // casting char to unsigend char
    unsigned char * ip_unsigned =  (unsigned char*) ip_next ; // casting char to unsigend char

   


    int port_parsed_back =  (port_unsigned[0]<< 8) +port_unsigned[1];
    unsigned long ip_parsed_back =  (ip_unsigned[0]<< 24) +(ip_unsigned[1]<< 16)+(ip_unsigned[2]<< 8)+(ip_unsigned[3]);
    char iparsed_in_long[INET_ADDRSTRLEN];
        // printf(" %ld \n",ip_parsed_back );


    // struct in_addr tmp ;
    
    // tmp.S_un.S_addr =ip_parsed_back ;
    // uint32_t ip_addr;
    //     memcpy(&ip_addr, &raw_packet[8], 4);
        // ip_parsed_back = ntohl(ip_parsed_back);        
        inet_ntop(AF_INET, &ip_parsed_back, iparsed_in_long, INET_ADDRSTRLEN);
        // packet->ip = (char*) calloc(INET_ADDRSTRLEN, sizeof(char));
        // memcpy(packet->ip, str, INET_ADDRSTRLEN);

    // printf("%s\n",iparsed_in_long );
    /*
        ****************sockets_intialising*****************
    */
    struct sockaddr_in SA ;
    bzero(&SA, sizeof(SA)); // putting zeros in it


    SA.sin_port = htons(port_parsed_back) ;
    if (flag_parse_portback != 1 )
    {
        SA.sin_port = htons(atoi(port_next)) ;
        memcpy( iparsed_in_long, ip_next,INET_ADDRSTRLEN);
    }

 
    SA.sin_family = AF_INET ;
    if ( isdigit(ip_next[0]))
    {
        SA.sin_addr.s_addr = inet_addr(iparsed_in_long) ;
    }
    else
    {

        struct hostent *info = gethostbyname(iparsed_in_long); // turn domain to hexi
        struct in_addr **listi;
                printf("port next is     %d\n", port_parsed_back);

        listi = (struct in_addr **) info->h_addr_list; //
                    printf("safe1     %d\n", port_parsed_back);

        SA.sin_addr.s_addr = inet_addr(inet_ntoa(*listi[0]));  // turn hexi to ip
    }// turn address to to binary
    // connecting
    if (connect(CS, (struct sockaddr *) &SA, sizeof(SA)) < 0)
        perror("cant connect client ") ;// socket, address of host , addres length

    if (send(CS, request, RequestSize, 0) == -1)
    {
        perror("send");
    }
    close(CS);


}



int handle_client(char *Buffer, char *answer, char *id_self,
                  char *ip__self,  char *port_self, DataItem_i **hashArray )
{


    int  index5 = 0 ;   // for get
    int  index6 = 0 ;  // for set
    int  index7 = 0 ;    // for delete

    index5 = Buffer[0]  & (1 << 2 ) ;  // get 1 bite if set to one index5 will be 4
    index6 = Buffer[0]  & (1 << 1 ) ;  // get 1 bite if set to one index6 will be 2
    index7 = Buffer[0]  & 1   ;   // get 1 bite if set to one index7 will be 1

    uint16_t   keylen = Buffer[3] + (Buffer[2] << 8 );       // copying the key  len
    uint16_t   datalen = Buffer[5] + (Buffer[4] << 8 );      // copying the data  len

    char tid = Buffer[1];


    char *key = NULL ;
    char *data = NULL ;
    key = Buffer + 14  ;
    data = key + keylen  ;
    char  keydebugguing[10000] ;
    char  datadebugguing[10000] ;
    strncpy(keydebugguing, &Buffer [14], keylen);
    keydebugguing[14 + keylen] = '\0' ;
    memcpy(datadebugguing, &Buffer[14 + keylen ], datalen);
    // datadebugguing[5+keylen+datalen] = '\0' ;

    bzero(answer, 10000) ;
    answer[0] = 8; // set akn
    answer[1] = tid ; // set id
    int  answersize = 14 ;  // only

    if (index5)
    {


        DataItem_i *requested = get(hashArray, HASHSIZE, keydebugguing, keylen  );
        if ( !requested )
        {
            perror("(get )key not found ")   ;
        }
        else
        {
            printf("retrived value succefully\n" );
            answer[0] = 12; // set get and akn

            answer[2] = (requested->keylen >> 8  & 0xFF);
            answer[3] =  requested->keylen   & 0xFF ; // set last 8 bits to 0
            answer[4] = (requested->datalen >> 8)  & 0xFF;
            answer[5] =  requested->datalen  & 0xFF;  //set last 8 bits to 0

            strncpy( &answer[14], requested->key, requested->keylen );
            memcpy( &answer[14 + requested->keylen ], requested->data, requested->datalen );
            answersize +=  requested->keylen ;
            answersize +=   requested->datalen ;
        }
        printf("%s  and  %d \n", keydebugguing, keylen );
    }
    else if (index6)
    {

        printf("%s  and  %d \n", keydebugguing, keylen );

        int r = set(hashArray, HASHSIZE, keydebugguing, data, keylen, datalen ) ;
        if (r == -1)
        {
            perror(" set hashtabele is full ")   ;
        }
        else if (r == 0)
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
        answer[0] = 10; // set set and akn



    }
    else if (index7)
    {
        //   for (int x = 0; x < i; ++x)
        // {
        //   printf("sicher1 ,   i ist  %d and  %c  \n",x,Buffer[x]);
        // }

        printf("%s  and  %d \n", keydebugguing, keylen );
        if (delete(hashArray, HASHSIZE, keydebugguing, keylen  ) == -1)
            perror("(delete )could not find the item to delete")   ;
        else
            printf("deleted succefully\n");
        answer[0] = 9; // set delete and akn

    }

    memcpy( &answer[6 ], & Buffer[6], 8 ); // copy ip port an id 


    answer[0]  = answer[0]  |(1 << 7);


    // for (int x = 0; x < answersize; ++x)
    //  {
    //     printf("%d and %d \n",x, answer[x]);
    //  }
    return answersize ;

}


void handle_stabalize(char *Buffer,char * ip__prev,char *port_prev , char *ip_next,
  char *port_next,char *id_prev, char *id_self, char *id_next)
{


		char id_to_send[10];
		char port_to_send[10];
        sprintf(id_to_send, "%d", ((unsigned char)Buffer[2] <<8) +  (unsigned char)Buffer[3]);
        unsigned long ip_parsed_back = (Buffer[4]<< 24) +(Buffer[5]<< 16)+(Buffer[6]<< 8)+(Buffer[7]);
        char iparsed_in_long[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &ip_parsed_back, iparsed_in_long, INET_ADDRSTRLEN);

        sprintf(port_to_send, "%d", ((unsigned char)Buffer[8] <<8) +  (unsigned char)Buffer[9]);



          if (!strlen(id_prev))
        {
            strcpy(id_prev,id_to_send);
            strcpy(ip__prev,iparsed_in_long);
            strcpy(port_prev,port_to_send);
            printf("node %s  updating it is predecessor to node %s with Ip %s an port %s  \n",
                id_self,id_prev , ip__prev, port_prev );

        }
    
    	
		Buffer[0] =  0 | 1<<7 | 1 << 5 ; // set internal and notify massage

    	/************************put my ip port and id inside */
        Buffer[2] =  atoi(id_prev) >> 8;
		Buffer[3] =  atoi(id_prev) ; 
    	unsigned long x;
     	x = inet_addr(ip__prev); // convert ip to long
    	Buffer[4] = x >> 24;
    	Buffer[5] = x >> 16;
    	Buffer[6] = x >> 8;
    	Buffer[7] = x ;
    	Buffer[8] =  atoi(port_prev) >> 8 ;
    	Buffer[9] =  atoi(port_prev) ;
	
        printf("node %s sending notify massage to node %s with ip %s an port %s   and the predecessor is %s\n",id_self,id_to_send, iparsed_in_long, port_to_send , id_prev );


        send_to_next_node(Buffer, 10, iparsed_in_long, port_to_send,0);

}

void handle_notify(char *Buffer,char * ip__prev,char *port_prev , char *ip_next,
  char *port_next,char *id_prev, char *id_self, char *id_next)
{	
	if (atoi(id_self)!= ((((unsigned char)Buffer[2])<<8 )+(unsigned char) Buffer[3] ))
	{
		/* code */
	
	    int id_request=  ((unsigned char)Buffer[2]<<8) + (unsigned char) Buffer[3]; 
		unsigned long ip_request =  (Buffer[4]<< 24) +(Buffer[5]<< 16)+(Buffer[6]<< 8)+(Buffer[7]);
    	int port_request= ( (unsigned char)Buffer[8]<<8) +(unsigned char) Buffer[9]; 
        

        sprintf(id_next, "%d", id_request);
        
       
        inet_ntop(AF_INET, &ip_request, ip_next, INET_ADDRSTRLEN);


    	sprintf(port_next, "%d", port_request);
    	printf("node %s is updating sucessor id to %s ip %s and port is %s \n",id_self , id_next , ip_next, port_next );



    }
}


void handle_join(char *Buffer,char * ip__prev,char *port_prev ,char * ip__self,char *port_self , char *ip_next,
  char *port_next,char *id_prev, char *id_self, char *id_next)
{	
	int flag_join  ; 
	int id_request= ((unsigned char)Buffer[2]<<8 )+ (unsigned char) Buffer[3]; 
    printf(" for join incoming id is %d and my id is  %d\n",id_request,atoi(id_self));
	if ( !strlen(id_next))
	{
		flag_join= 0 ;
        printf("case1\n");
	}
	else if (!strlen(id_prev) && strlen(id_next))
	{	
		flag_join= 1 ;
		if (id_request <= atoi(id_self)  ) // IF MINE 
			flag_join= 0 ;
                printf("case2\n");


	}
	else if (strlen(id_prev)  && strlen(id_next) )
	{	
		flag_join= 1 ;
		 if ((id_request <= atoi(id_self) && id_request > atoi(id_prev)) || ( atoi(id_prev) >atoi(id_self)
		  && id_request > atoi(id_prev) && id_request > atoi(id_self))
        	|| (atoi(id_prev) >atoi(id_self) && id_request >= 0 && id_request < atoi(id_self)) )
	    	flag_join= 0;
                printf("case3\n");


	    
	}
	printf("flag join is %d\n",flag_join );
	if (!flag_join)
	{	
        
    	int id_request= ((unsigned char)Buffer[2]<<8) + (unsigned char) Buffer[3]; 
		unsigned long ip_request =  (Buffer[4]<< 24) +(Buffer[5]<< 16)+(Buffer[6]<< 8)+(Buffer[7]);
    	int port_request=( (unsigned char)Buffer[8]<<8) +(unsigned char) Buffer[9]; 
    	sprintf(id_prev, "%d", id_request);
    	// printf("ip request in long  %ld\n",ip_request );


    	inet_ntop(AF_INET, &ip_request, ip__prev, INET_ADDRSTRLEN);
    	sprintf(port_prev, "%d", port_request);
        printf("node %s  updating it is predessecor to node %s with Ip %s an port %s  \n",id_self
        ,id_prev, ip__prev, port_prev );

         if (!strlen(id_next))
        {
            strcpy(id_next,id_prev);
            strcpy(ip_next,ip__prev);
            strcpy(port_next,port_prev);
            printf("node %s  updating it is successor to node %s with Ip %s an port %s  \n",id_self,id_next
            , ip_next, port_next );

        }

    	Buffer[0] =  0 | 1<<7 | 1 << 5 ; // set internal and notify massage

    	/************************put my ip port and id inside */
        Buffer[2] =  atoi(id_self) >> 8;
		Buffer[3] =  atoi(id_self) ; 
    	unsigned long x;
     	x = inet_addr(ip__self); // convert ip to long
    	Buffer[4] = x >> 24;
    	Buffer[5] = x >> 16;
    	Buffer[6] = x >> 8;
    	Buffer[7] = x ;
    	Buffer[8] =  atoi(port_self) >> 8 ;
    	Buffer[9] =  atoi(port_self) ;

		printf("node %s sending notify massage to node %s ip is %s and port is %s \n"
    			,id_self,id_prev ,ip__prev,port_prev);
        send_to_next_node(Buffer, 10, ip__prev, port_prev,0); //send notify amssage back so node updates it,s sucessor


	}
	else // send to next node if not in my ares
	{	
        send_to_next_node(Buffer, 10, ip_next, port_next,0);

	}
                            	


	
	
}
int main(int argc, char **args)
{

    /************************   parse input     *************************/

    /*if (argc < 9 || args[1] == NULL || args[2] == NULL || args[3] == NULL || args[4] == NULL
            || args[5] == NULL || args[6] == NULL || args[7] == NULL || args[8] == NULL || args[9] == NULL)
    {
        printf("error please correct input  ") ;
        exit(1);
    }*/

    for (int i = 0; i < argc; ++i)
    {
    	if (!strcmp( args[i],"localhost"))
    		 args[i] = "127.0.0.1" ;
        
    }
    char ip__self[20];
    char id_self[20];
    char port_self[20] ;
    char ip__prev[20];
    char id_prev[20];
    char port_prev[20] ;
    char ip_next[20];
    char id_next[20];
    char port_next[20];


    int Flag_case = -1 ; // flag to know how far A pper right now is 
    printf("%d\n",argc );
    if (argc ==2 ||argc ==3||argc ==4)
    {
	    strcpy(ip__self , args[1]);
	    strcpy(port_self  ,args[2])  ;
	    strcpy(id_self , args[3]);
	    if (!strlen(id_self) )  // if no optional id the nwrite 0 inside 
	   	    strcpy(id_self, "0");
	   	Flag_case = 0 ; //set todo nothing
	   	printf("it is case 1 \n");

       

    }
    else if (argc ==6 ||argc==7)
    {	
    	strcpy(ip__self , args[1]);
	   strcpy( port_self , args[2])  ;
	    strcpy(id_self , args[3]);
	   strcpy( ip_next  , args[4]);
	   strcpy( port_next , args[5])  ;
      
    

    	Flag_case = 1 ; //set to join massage 

    }
    else
    {
        strcpy(id_self ,args[1]);
        strcpy(ip__self , args[2]);
        strcpy(port_self  , args[3])  ;
	    strcpy(id_prev  ,args[4]);
	    strcpy(ip__prev , args[5]);
	    strcpy(port_prev , args[6]);
	    strcpy(id_next , args[7]);
	    strcpy(ip_next  , args[8]);
	    strcpy(port_next , args[9])  ;

    }

    



    /************************   making connections  *************************/


    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char Buffer[10000];    // buffer for client data
    int nbytes; // for recv return value
    char answer[10000];    // buffer for answer data
    int answersize = 0 ;
    char newprotococlient[10000];
    int newprotococlientsize = 0 ;
    char answerClientProtocol[10000];
    int answerClientProtocolSize = 0 ;



    char remoteIP[INET6_ADDRSTRLEN];    // can be changed

    int yes = 1;      // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;  // i iterate aover all fd j iterate over master
    // rv for return value frm get aaddress info

    struct addrinfo hints, *ai, *p; // ahints for intiallising i gives lis tp iterat over it

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM ;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, port_self, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    // opening  a socket for ever address in ai
    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(AF_INET,  SOCK_STREAM | SOCK_NONBLOCK, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR  , &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL)
    {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1)
    {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop

    /************************   adding connections and parsing requsts      *************************/


    DataItem_i * *hashArray = erstelle(HASHSIZE); // create hash table
    DataItem_i * *clientsid = erstelle(CLIENTSSIZE); // create hash table



    if (Flag_case ==1)
    {
    	char Notify_massage[10] ;
    	Notify_massage[0] = 0 | 1 << 7 |1 << 6 ; // set join and internal bit 
    	Notify_massage[1] = 0 ; 
    	Notify_massage[2] =  atoi(id_self) >> 8;
    	Notify_massage[3] =  atoi(id_self) ; 
    	unsigned long x;
    	x = inet_addr(ip__self); // convert ip to long
    	Notify_massage[4] = x >> 24;
    	Notify_massage[5] = x >> 16;
    	Notify_massage[6] = x >> 8;
    	Notify_massage[7] = x ;
    	Notify_massage[8] =  atoi(port_self) >> 8 ;
    	Notify_massage[9] =  atoi(port_self) ;
    	send_to_next_node( Notify_massage,10, ip_next,  port_next,0);


    }
 //    int timer = 4; /* 10ms */
	// clock_t start = clock();
 //    clock_t d;
    struct timeval        timeout;
    timeout.tv_sec  = 4;
    timeout.tv_usec = 0;

    for (;;)
    {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, &timeout) == -1)
        {
            perror("select");
            exit(4);
        }else if(!select(fdmax + 1, &read_fds, NULL, NULL, &timeout)
            && strlen(ip_next) && strlen(port_next))
        {   
            timeout.tv_sec  = 10;
            timeout.tv_usec = 0;
            char stablize_massage[10];

            stablize_massage[0] =  0 | 1<<7 | 1 << 4 ; // set internal and stablize massage

            /************************put my ip port and id inside */
            stablize_massage[2] =  atoi(id_self) >> 8;
            stablize_massage[3] =  atoi(id_self) ; 
            unsigned long x;
            x = inet_addr(ip__self); // convert ip to long
            stablize_massage[4] = x >> 24;
            stablize_massage[5] = x >> 16;
            stablize_massage[6] = x >> 8;
            stablize_massage[7] = x ;
            stablize_massage[8] =  atoi(port_self) >> 8 ;
            stablize_massage[9] =  atoi(port_self) ;
            printf("node %s is sending stabalize massage to node %s with ip %s and port %s \n",id_self,id_next,ip__self,port_next );
            send_to_next_node(stablize_massage, 10, ip_next, port_next,0);
            sleep(2); //give time to response
        }

         // d = clock() - start;
         // printf("time is %ld \n",d);

 		// d = d / CLOCKS_PER_SEC;
   //      if ( d >= timer )  // send stabalize m ssage 
	  //   {
	  //   	start = clock();// reset timer 
	    	

	  //   }

        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++)
        {   


            if (FD_ISSET(i, &read_fds))   // we got one!!
            {
                if (i == listener)
                {
                    // handle new connections
                    addrlen = sizeof remoteaddr;    //  addresslen = socket len  , remote add == sockaddr_storage
                    newfd = accept(listener,            // newfd ist the socket of the other connection point
                                   (struct sockaddr *)&remoteaddr,
                                   &addrlen);

                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax)      // keep track of the max
                        {
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,          // save remote addr als string in remoteIP with length INet6 an from family sss.family
                                         get_in_addr((struct sockaddr *)&remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN), newfd); //on the sockt newfd
                    }
                }
                else
                {
                    // handle data from a client
                    if ((nbytes = recv(i, Buffer, sizeof Buffer, 0)) <= 0)
                    {
                        // got error or connection closed by client
                        if (nbytes == 0)
                        {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    }
                    else
                    {
                    	// only if set or get or delete bits set then handle it as a client else always join operations 
                    	if (CHECK_BIT(Buffer[0], 2)== 4||CHECK_BIT(Buffer[0], 1)== 2||CHECK_BIT(Buffer[0], 0)== 1)
                    	{
                    	    if (CHECK_BIT(Buffer[0], 1)== 2) // stop sending stablize if the first set order have arrived 
                            {
                                timeout.tv_sec  = 99999999;
                                timeout.tv_usec = 0;
                            }

	                        // we got some data from a connection
	                        int check_action = -1 ;
	                        int client_not_mine_already_sent = -1 ; // flag so don,t send client atwo time if already sen t 

	                        if (!(Buffer[0] >> 7)) // it is a client
	                        {
	                            convert_client_protocol(Buffer, newprotococlient, id_self, ip__self,   port_self);
	                            newprotococlientsize = nbytes + 8 ; // 8 = 4 id + ip +port
	                            /***************putting the id in my list  **********/
	                            char *clientindex = (char*)malloc(sizeof(char) * 1)  ;
	                            sprintf(clientindex, "%d", i);
	                            char *clientid = (char*)malloc(sizeof(char) * 4)  ;
	                            sprintf(clientid, "%d", Buffer[1]);


	                            int r = set(clientsid, CLIENTSSIZE, clientid, clientindex, strlen(clientid), 1 ) ;

	                            if (r == -1)
	                            {
	                                perror(" set hashtabele is full ")   ;
	                            }
	                            memcpy( Buffer , newprotococlient , newprotococlientsize);
	                            nbytes= newprotococlientsize;

	                        }
	                       for (int i = 0; i < nbytes ; ++i)
	                            {
	                                printf(" nonde =  %d  request iafter hanndeling is  ,   i ist  %d and  %d  \n",atoi(id_self),i,Buffer[i]);
	                            }
	                       


	                        // for (int i = 0; i < newprotococlientsize ; ++i)
	                        // {
	                        //     printf("request iafter hanndeling is  ,   i ist  %d and  %d  \n",i,Buffer[i]);
	                        // }
	                        /***************checking if the client s my responsiplaity **********/
	                        uint16_t   keylen = Buffer[3] + (Buffer[2] << 8 );       // copying the key  le
	                       if (nbytes >14) //if aknowladgment then skip 
	                       {
	                        check_action = hashingclient(Buffer, keylen, DHTSIZE, id_prev, id_self, id_next); // return 1 if mines
	                       }
	                        char *clientid = (char*)malloc(sizeof(char) * 4)  ;
	                        sprintf(clientid, "%d", Buffer[1]);  // prepa to check if that a client in my list
	                        if (check_action == 1) // if it is mine then go in  
	                        {
	                        
	                                printf("node %d is going in \n",atoi(id_self));                                
	                            
	                            answersize = handle_client(Buffer, answer, id_self, ip__self,   port_self, hashArray);

	                            memcpy( Buffer , answer , answersize);// reaplace buffer with the answer
	                            int id_requesting = ( Buffer[6] << 8) + Buffer[7] ;
	                            // printf("id recuestinf is and  %d  \n",id_requesting);
	                            for (int i = 0; i < answersize ; ++i)
	                            {
	                                printf(" node =  %d  answer in the end is   %d and  %d  \n",atoi(id_self),i,Buffer[i]);
	                            }
	                            
	                            if (id_requesting != atoi(id_self)) // if  not  the self
	                            {
	                                char ip_requesting[4];
	                                 ip_requesting[0]=Buffer[8];
	                                 ip_requesting[1]=Buffer[9];
	                                 ip_requesting[2]=Buffer[10];
	                                 ip_requesting[3]=Buffer[11];
	                                // memcpy(ip_requesting, &Buffer[8], 4);
	                                char port_requesting[2];
	                                 port_requesting[0]=Buffer[12];
	                                 port_requesting[1]=Buffer[13];

	                                // memcpy(port_requesting, &Buffer[12], 2);

	                            // printf("answer ip 1 %d\n", ip_requesting[0]);
	                            // printf("answer ip 2    %d\n", ip_requesting[1]);
	                            // printf("answer ip 3    %d\n", ip_requesting[2]);
	                            // printf("answer ip 4    %d\n", ip_requesting[3]);
	                            // printf("answer port 1    %d\n", port_requesting[0]);
	                            // printf("answer port 2    %d\n", port_requesting[1]);
	                                Buffer[6] = atoi(id_self) >> 8 ;
	                                Buffer[7] = atoi(id_self) ;

	                                long x;
	                                x = inet_addr(ip__self); // convert ip to long
	                                Buffer[8] = x >> 24 ;
	                                Buffer[9] = x >> 16 ;
	                                Buffer[10] = x >> 8 ;
	                                Buffer[11] = x ;

	                                Buffer[12] = atoi(port_self) >> 8 ;
	                                Buffer[13] = atoi(port_self) ;


	                                client_not_mine_already_sent = 1  ;
	                              
	                                printf("node %d is putting his own connection info and sending to back to request node    \n",atoi(id_self));                                
	                            
	                                send_to_next_node(Buffer, answersize, ip_requesting, port_requesting,1);
	                            }



	                        }
	                        else if((Buffer[0] >> 7) == -1 && !CHECK_BIT(Buffer[0], 3) )
	                        {
	                            
	                            printf("node %d is sending request to next  node   %d  \n",atoi(id_self),atoi(id_next));                                


	                            client_not_mine_already_sent = 1  ;
	                            send_to_next_node(Buffer, nbytes, ip_next,  port_next,0);
	                        }
	                        // printf("check bit answer is  %d\n",CHECK_BIT(Buffer[0], 3) );
	                        // checking if akn is et 

	                        // printf("safe1\n");

	                        /************* if the answer is for my client*********/
	                       

	                        // if akn bit set and client not already sen t and it is an internal message and this id is in my cliebts list then send back to client 
	                        if (CHECK_BIT(Buffer[0], 3)== 8 && client_not_mine_already_sent!= 1 && (Buffer[0] >> 7) == -1 && get(clientsid, CLIENTSSIZE, clientid, strlen(clientid)))
	                        {
	                            // printf("safe2\n");
	                            printf("node %d thinks clientis his  \n",atoi(id_self));                                
	                           


	                            DataItem_i *requested = get(clientsid, CLIENTSSIZE, clientid, strlen(clientid) );
	                            if ( !requested )
	                            {
	                                perror("(get )key not found ")   ;
	                            }
	                            else
	                            {
	                                printf("retrived value succefully\n" );
	                            }
	                            for (j = 0; j <= fdmax; j++)
	                            {
	                                // send back to client !

	                                // for (int i = 0; i < answersize ; ++i)
	                                // {
	                                //     printf("request iafter hanndeling is  ,   i ist  %d and  %d  \n",i,Buffer[i]);
	                                // }

	                                if (FD_ISSET(j, &master) && atoi(requested->data) == j)
	                                {
	                                    // except the listener and ourselves
	                                    answerClientProtocolSize = convert_client_protocol_back(answerClientProtocol, Buffer);
	                                    if (j != listener )
	                                    {   
	                                        printf("node %dis sending result to client in i = %d \n",atoi(id_self),j);                                

	                                        if (send(j, answerClientProtocol, answerClientProtocolSize, 0) == -1)
	                                        {
	                                            perror("send");
	                                        }
	                                    }
	                                }

	                            }   
	                        }

                    	} //  if case  client 
                    	else 
                    	 // case join 
                    	{
                    		 // for (int i = 0; i < nbytes ; ++i)
	                      //       {
	                      //           printf(" nonde =  %d  request join is  ,   i ist  %d and  %d  \n",atoi(id_self),i,Buffer[i]);
	                      //       }
	                            // printf("%d      %d\n",CHECK_BIT(Buffer[0], 6),(Buffer[0] >> 7)  );
                    		if (CHECK_BIT(Buffer[0], 6)== 64 &&(Buffer[0] >> 7) == -1) // create a new join massage
	                        {	//raeturns  0 if mins and 1 if should forward 
	                           printf("it is join request\n");
                               // id_prev = (char*)malloc(20 *sizeof(char) );
                               // ip__prev = (char*)malloc(20 *sizeof(char) );
                               //  port_prev = (char*)malloc(20 *sizeof(char) );

	                           handle_join(Buffer, ip__prev,port_prev,ip__self,port_self,ip_next,
	                           port_next, id_prev, id_self, id_next); // reutrn 1 if responible else 0
	                            
	                        }
	                        else if (CHECK_BIT(Buffer[0], 5)== 32 &&(Buffer[0] >> 7) == -1) // create a new join massage
	                        {	
	                           printf("it is notify request\n");
                                // id_next = (char*)malloc(20 *sizeof(char) );
                                // ip_next = (char*)malloc(20 *sizeof(char) );
                                // port_next = (char*)malloc(20 *sizeof(char) );

	                           handle_notify(Buffer, ip__prev,port_prev,
	                           ip_next, port_next,id_prev, id_self, id_next); // reutrn 1 if responible else 0
	                        }
	                        else if (CHECK_BIT(Buffer[0], 4)== 16 &&(Buffer[0] >> 7) == -1) // create a new join massage
	                        {	
	                           printf("it is stablize request\n");

	                           handle_stabalize(Buffer, ip__prev,port_prev,
	                           ip_next, port_next,id_prev ,id_self, id_next); // reutrn 1 if responible else 0

	                        }
                    	}

                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!







    return 0;
}

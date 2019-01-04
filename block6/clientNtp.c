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
#include <sys/time.h>
#include <math.h>
#include <inttypes.h>
#include <float.h>
#include <limits.h>

struct server 
{
     char* link  ;
     double roundTripMax ;
     double roundTripMin ;
     double disprion;
	 double rootDisprion ;
	 int stratum   ;
	 int reliableFlag ;
	 double   OffSet ;
    
};
typedef struct server server_i;

int getBestServer(server_i ** serverList)
{
	double mindisperion = DBL_MAX ; 
	int bestIndex = 0; 
	for (int i = 0; i < 3; ++i)
	{	
		double  currentdisperion =  (serverList[i]->roundTripMax - serverList[i]->roundTripMin) + serverList[i]->rootDisprion ;
		if(currentdisperion< mindisperion )
		{	
			mindisperion= currentdisperion ;
			bestIndex= i ; 
		}

		// printf("currentdisperionis %f \n",currentdisperion);
		// printf("rootdisperion %f \n",serverList[i]->rootDisprion);

	}
	return bestIndex ;
}
void parseSrverAnswer(char * serverAnswer, server_i * tmpServer ,double originalStampleResult ,double referenceStampleResult)
{
 /*************** get T3 T4***************/
;

	unsigned long long  ReceiveStampleResultsec = (unsigned char)serverAnswer[32];
	ReceiveStampleResultsec = ReceiveStampleResultsec <<8 ;
	ReceiveStampleResultsec += (unsigned char)serverAnswer[33] ;
	ReceiveStampleResultsec = ReceiveStampleResultsec <<8 ;
	ReceiveStampleResultsec += (unsigned char)serverAnswer[34] ;
	ReceiveStampleResultsec = ReceiveStampleResultsec <<8 ;
	ReceiveStampleResultsec += (unsigned char)serverAnswer[35] ;
	ReceiveStampleResultsec -= 2208988800 ;


	unsigned long long ReceiveStampleResultmillisec = (unsigned char)serverAnswer[36] ;
	ReceiveStampleResultmillisec = ReceiveStampleResultmillisec <<8 ;
	ReceiveStampleResultmillisec += (unsigned char)serverAnswer[37] ;
	ReceiveStampleResultmillisec = ReceiveStampleResultmillisec <<8 ;
	ReceiveStampleResultmillisec += (unsigned char)serverAnswer[38] ;
	ReceiveStampleResultmillisec = ReceiveStampleResultmillisec <<8 ;
	ReceiveStampleResultmillisec += (unsigned char)serverAnswer[39] ;

	double  ReceiveStampleResult = ((double) ReceiveStampleResultsec ) + ((double) ReceiveStampleResultmillisec * 1.0e-10) ;
	// printf("ReceiveStample  %f\n",ReceiveStampleResult);

	// merging millsec with sec 
	unsigned long long  TransmitStampleResultsec = (unsigned char)serverAnswer[40];
	TransmitStampleResultsec = TransmitStampleResultsec <<8 ;
	TransmitStampleResultsec += (unsigned char)serverAnswer[41] ;
	TransmitStampleResultsec = TransmitStampleResultsec <<8 ;
	TransmitStampleResultsec += (unsigned char)serverAnswer[42] ;
	TransmitStampleResultsec = TransmitStampleResultsec <<8 ;
	TransmitStampleResultsec += (unsigned char)serverAnswer[43] ;
	TransmitStampleResultsec -= 2208988800 ;
	
	unsigned long long  TransmitStampleResultmillisec = (unsigned char)serverAnswer[44];
	TransmitStampleResultmillisec = TransmitStampleResultmillisec <<8 ;
	TransmitStampleResultmillisec += (unsigned char)serverAnswer[45] ;
	TransmitStampleResultmillisec = TransmitStampleResultmillisec <<8 ;
	TransmitStampleResultmillisec += (unsigned char)serverAnswer[46] ;
	TransmitStampleResultmillisec = TransmitStampleResultmillisec <<8 ;
	TransmitStampleResultmillisec += (unsigned char)serverAnswer[47] ;
	double  TransmitStampleResult = ((double) TransmitStampleResultsec  )+((double) TransmitStampleResultmillisec * 1.0e-10) ;

	// printf("TransmitStample  %f\n",TransmitStampleResult);

 /*************** calc offset , rtn and rootDisprion**************/
	tmpServer->stratum =serverAnswer[1]; 
	// calc root deisperion 
	tmpServer->rootDisprion =(((unsigned char)serverAnswer[8] <<24)+ ((unsigned char)serverAnswer[9] <<16) 
	 + ((unsigned char)serverAnswer[10] <<8) + (unsigned char)serverAnswer[11] ) ;
	
	double rtb = (  referenceStampleResult - originalStampleResult ) 	- ( TransmitStampleResult - ReceiveStampleResult   ) ;
	// printf("round trip is  %f\n",rtb);


	if( rtb > tmpServer->roundTripMax )
		tmpServer->roundTripMax = rtb ;
	if( rtb < tmpServer->roundTripMin )
	{
		tmpServer->roundTripMin = rtb ;
		tmpServer->OffSet = ((ReceiveStampleResult - originalStampleResult ) + (TransmitStampleResult - referenceStampleResult ))/2 ;
		// printf("offset is  %f\n",tmpServer->OffSet);
	}


}
int main(int argc, char** args)
{


/*
	****************preparing serverList*****************
*/


	server_i * serverlist[3] ;

	serverlist[0] = (server_i*) malloc(sizeof(server_i)*1);
	serverlist[0]->link = "stratum2-4.NTP.TechFak.Uni-Bielefeld.DE"  ;
	serverlist[0]->roundTripMax =  DBL_MIN ;
	serverlist[0]->roundTripMin =  DBL_MAX ;
	serverlist[0]->reliableFlag =  0;

	serverlist[1] = (server_i*) malloc(sizeof(server_i)*1);
	serverlist[1]->link = "ntp0.rrze.uni-erlangen.de"  ;
	serverlist[1]->roundTripMax =  DBL_MIN ;
	serverlist[1]->roundTripMin =  DBL_MAX  ;
	serverlist[1]->reliableFlag =  0;
	
	serverlist[2] = (server_i*) malloc(sizeof(server_i)*1);
	serverlist[2]->link = "time1.uni-paderborn.de" ; 
	serverlist[2]->roundTripMax =  DBL_MIN ;
	serverlist[2]->roundTripMin =  DBL_MAX ;
	serverlist[2]->reliableFlag =  0;


/*
	****************prebaring client request*****************
*/

	char clientRequest[48] ;
	clientRequest[0] = 35;
	char serverAnswer[48] ;

/*
	****************sockets_create*****************
*/

		
	for (int i = 0; i < 3; ++i)
	{


		int CS = socket(AF_INET,SOCK_DGRAM,0);  // creating client socket 
		if(CS<0)
			perror("cant make client socket") ;

	/*
		****************sockets_intialising*****************
	*/

		// printf("connecting to server %s\n", serverlist[i]->link);
		struct sockaddr_in SA ;

		bzero(&SA,sizeof(SA)); // putting zeros in it 

	 	SA.sin_port = htons(123) ;
		SA.sin_family = AF_INET ;

	    struct hostent* info=gethostbyname(serverlist[i]->link);// turn domain to hexi 
	    struct in_addr **listi;
	    listi = (struct in_addr **) info->h_addr_list; //
	    SA.sin_addr.s_addr = inet_addr(inet_ntoa(*listi[0]));  // turn hexi to ip 

		// getting t1


		struct timeval originalStample ,referenceStample;

		//TODO : adjust BAck to 8
		for (int j = 0; j < 4; ++j) // do 8 times to calculate best disperiona
		{
			gettimeofday(&originalStample, NULL);
			double originalStampleResult  = ( (double) originalStample.tv_sec) + ((double) (double) originalStample.tv_usec *1.0e-6 ) ;
			// printf("originalStample %f\n", originalStampleResult);
			// GET T1
			sendto(CS, (const char *)clientRequest, 48,                 
		        0, (const struct sockaddr *) &SA,  
		            sizeof(SA));           // implicit bind 


			int answerlen= 0;
			int init = 0;
			while(answerlen< 48)
			{	
				answerlen += recvfrom(CS, serverAnswer, 48, 0, (struct sockaddr*) &SA, (socklen_t *)&init);
				if( answerlen < 0 )  // recieiving für udp
					printf("error recieiving\n");
			}
			// for (int x = 0; x < 48; ++x)
		 //     {
		 //        printf("%d and %d \n",x, (unsigned char) serverAnswer[x]);
		 //     }

		    gettimeofday(&referenceStample, NULL);
			double referenceStampleResult  = ((double) referenceStample.tv_sec) + ((double) referenceStample.tv_usec *1.0e-6 ) ;
			// printf("referenceStample %f\n",referenceStampleResult);

			// GET T2 
			parseSrverAnswer(serverAnswer,serverlist[i],originalStampleResult,referenceStampleResult);
			sleep(0.5);

		}

		close(CS);
	}
		int bestServerIndex = getBestServer(serverlist);
		printf("bestServer to sychronize to  is %s\n", serverlist[bestServerIndex]->link);
		/************printing real time************/
		// geting sec 		

		struct timeval realtime;
		gettimeofday(&realtime, NULL);

		// turning  sec into a readable form	
		struct tm* timesec = localtime(&realtime.tv_sec);
		
		// geting millisec 		
		int millisec = lrint(realtime.tv_usec/1000.0); 
		if (millisec>=1000) { 
		   millisec -=1000;
		   realtime.tv_sec++;
		 }
		 // printing
		char realtimeSeconds[26]; // for part without millis
		strftime(realtimeSeconds, 26, "%Y:%m:%d %H:%M:%S", timesec);
		printf("Local Clock is %s.%03d\n", realtimeSeconds, millisec);

		/*********printing offset************/		

		printf("OffSet is  %f \n", serverlist[bestServerIndex]->OffSet);
		
		/*********correcting the clock************/		
		// correct clock all 
		double correctTime= (double) realtime.tv_sec + ((double) realtime.tv_usec *1.0e-6 ) +serverlist[bestServerIndex]->OffSet ;
		
		struct timeval correctClock ;
		// calc sec 
		correctClock.tv_sec =( unsigned long long) correctTime ;
		// calc microsec
		correctClock.tv_usec =( unsigned long long)  ((correctTime -(double )correctClock.tv_sec) *1.0e6);
		// turn sec to a gut format
		struct tm* correctsec = localtime(&correctClock.tv_sec);


		int correctMillisec = lrint(correctClock.tv_usec/1000); 
		if (correctMillisec>=1000) { 
		   correctMillisec -=1000;
		   correctClock.tv_sec++;
		 }
		char correctTimeSeconds[26]; // for part without millis
		strftime(correctTimeSeconds, 26, "%Y:%m:%d %H:%M:%S", correctsec);
		printf("Correct Clock is %s.%03d\n", correctTimeSeconds, correctMillisec);




}

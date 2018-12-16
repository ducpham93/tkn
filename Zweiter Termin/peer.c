#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include "uthash.h"

char* result;
char* keyc;
char* valuec;
char* normalresp;
char headpeer_ip[12];
char* headpeer_port;
char suc_ip[12];
char pre_ip[12];
char myip[4];
int preipset;

char* short2string(unsigned short port)
{
	float tmp;
	int mean;
	if (port >= 10000){
		tmp = (float)port/10000;
		result[0] = (int)tmp + 48;
		mean = port - (int)tmp*10000;
		tmp = mean/1000;
		result[1] = (int)tmp + 48;
		mean = mean - (int)tmp*1000;
		tmp = mean/100;
		result[2] = (int)tmp + 48;
		mean = mean - (int)tmp*100;
		tmp = mean/10;
		result[3] = (int)tmp + 48;
		mean = mean - (int)tmp*10;
		tmp = mean;
		result[4] = (int)tmp + 48;
	}
	else {
		tmp = (float)port/1000;
		result[1] = (int)tmp + 48;
		mean = port - (int)tmp*1000;
		tmp = mean/100;
		result[2] = (int)tmp + 48;
		mean = mean - (int)tmp*100;
		tmp = mean/10;
		result[3] = (int)tmp + 48;
		mean = mean - (int)tmp*10;
		tmp = mean;
		result[4] = (int)tmp + 48;
	}	
	return result;
}	

typedef struct video video;

struct video 
{
    char* id;
    char* value;             
    UT_hash_handle hh;
};

video *database = NULL;

void add_user(video *s) 
{  
	HASH_ADD_KEYPTR( hh, database, s->id, strlen(s->id), s );
}

video* find_user(char* movietitle) 
{
    video *s;
    HASH_FIND_STR(database, movietitle, s);  
	return s;	
}

video* new_video(char* id, char* value)
{
	video* new = (video*) calloc(1, sizeof(video));
	new->id = id;
	new->value = value;
	return new;
}

void delete_all() 
{
  struct video *current_user, *tmp;

  HASH_ITER(hh, database, current_user, tmp) {
    HASH_DEL(database,current_user);
    free(current_user);
  }
}	

void print_users() 
{
    struct video *s;

    for(s=database; s != NULL; s=s->hh.next) {
        printf("key %s: value %s\n", s->id, s->value);
    }
}

void delete_user(video* data) {
    HASH_DEL(database, data);
	free(data);
}

void ipchar_to_string(char ip[4])
{ 
	for (int j = 0; j < 12; j++){
		headpeer_ip[j] = 0;
	}	
    snprintf(headpeer_ip, 12, "%d.%d.%d.%d", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]); 
}

void ipchar_to_string2(char ip[4])
{ 
	for (int j = 0; j < 12; j++){
		suc_ip[j] = 0;
	}
    snprintf(suc_ip, 12, "%d.%d.%d.%d", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]); 
}

void ipchar_to_string3(char ip[4])
{ 
	for (int j = 0; j < 12; j++){
		pre_ip[j] = 0;
	}
    snprintf(pre_ip, 12, "%d.%d.%d.%d", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]);
	preipset = 1;
}

void parseIPV4string(char* ipAddress, char ipbytes[4]){
	int tmp[4];
	sscanf(ipAddress, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
	for(int k = 0; k < 4; k++){
		ipbytes[k] = (char) tmp[k];
	}
}

void parseIPV4stringmyip(char* ipAddress){
	int tmp[4];
	sscanf(ipAddress, "%d.%d.%d.%d", &tmp[3], &tmp[2], &tmp[1], &tmp[0]);
	for(int k = 0; k < 4; k++){
		myip[k] = (char) tmp[k];
	}
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

unsigned short char2short(unsigned char pchar[2])
{
	unsigned short pshort;
	pshort = (((short)pchar[0]) << 8) | pchar[1];
	return pshort;
}

int between(unsigned short lower, unsigned short upper, unsigned short middle){
	if (lower < upper){
		if (middle > lower && middle <= upper){
			return 1;
		}
		else {
			return 0;
		}
	}
	else if (lower > upper){
		if (middle > lower || middle <= upper){
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if (middle == upper){
			return 1;
		}
		else {
			return 0;
		}
	}	
}

unsigned short power(int x,int n)
{
    /* Variable used in loop counter */
    unsigned short number = 1;

    for (int j = 0; j < n; ++j)
        number *= x;

    return(number);
}

unsigned short self_id;

typedef struct _ftentry {
    unsigned short start;
    unsigned short successor;
    unsigned short port;
    char ip[12];
    struct _ftentry *next;
} ftentry;

ftentry* head;

ftentry* new_entry(unsigned short i){
	ftentry* element = (ftentry*) calloc(1, sizeof(ftentry));
	element->start = (power(2,i) + self_id) % 256;
	element->successor = 0;
	element->port = 0;
	for (int j = 0; j < 12; j++){
		element->ip[j] = 0;
	}	
	element->next = NULL;
	return element;
}

void constructft(){
	head = new_entry(0);
	ftentry* tmp = head;
	for(int j = 1; j < 8; j++){
		tmp->next = new_entry(j);
		tmp = tmp->next;
	}	
	printf("My fingertable starting points:\n");
	for (ftentry* tmp = head; tmp != NULL; tmp = tmp->next){
		printf("%d\n", tmp->start);
	}
}

ftentry* lookupft(int id){
	ftentry* tmp;
	for (tmp = head; tmp != NULL; tmp = tmp->next){
		if (id == tmp->start){
			break;
		}
	}
	return tmp;
}	

void updateft(unsigned short id, unsigned short suc, unsigned short p, char ip[12]){
	for (ftentry* tmp = head; tmp != NULL; tmp = tmp->next){
		if (id == tmp->start){
			tmp->successor = suc;
			tmp->port = p;
			for (int j = 0; j < 12; j++){
				tmp->ip[j] = ip[j];
			}
		}
	}	
}	

void printft(){
	ftentry* tmp;
	for (tmp = head; tmp != NULL; tmp = tmp->next){
		printf("Start: %d\t Successor: %d\t Port: %d\n", tmp->start, tmp->successor, tmp->port);
	}
}	
	
int main(int argc, char *argv[])
{
	if (argc != 3 && argc != 4 && argc != 6) {
        fprintf(stderr,"usage: peer ip port (id) (contact-peer-ip contact-peer-port)\n");
        exit(1);
    }
	
	self_id = 0;
	
	for (int j = 0; j < 12; j++){
		pre_ip[j] = 0;
		suc_ip[j] = 0;
	}	
	
	if (argc == 4){
		self_id = atoi(argv[3]);
		
	}
	else if (argc == 6){
		self_id = atoi(argv[3]);
	}
	
	unsigned short pre_id = self_id; 
	unsigned short suc_id = self_id;
	unsigned short headid;
	unsigned short self_port = atoi(argv[2]);
	unsigned short suc_p = 0;
	unsigned short pre_p = 0;
	char* suc_port;
	char* pre_port;
	char* ft_port;
	preipset = 0;
	
	constructft();
	
	printf("\n");
	printf("Hi, my peer ID is %d, how may I help you?\n", self_id);
    fd_set master;
    fd_set read_fds;
    int fdmax;
	unsigned short kli, vli, headpeer_p, requestID;
	short get, set, del, ack, join, notify, stabilize, internal;
	int clientfd, idkey;
	//int stabilizeresponse;

    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
	char stabmessage[14];

    char buf[1000];
    int nbytes;
	int headlineclient[8];
	char byte;

    int yes=1;
    int i, rv;

	//int sockfdd;
	int sockfiled[10];
	struct addrinfo hintss, *servinfoo, *pp;
	struct addrinfo contacthintss, *contactservinfoo, *contactpp;
	struct addrinfo hints, *ai, *p;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, argv[2], &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}
	//fcntl(listener, F_SETFL, O_NONBLOCK);
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}
	freeaddrinfo(ai);
    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }
	if (argc == 6){
		memset(&contacthintss, 0, sizeof contacthintss);
		contacthintss.ai_family = AF_UNSPEC;
		contacthintss.ai_socktype = SOCK_STREAM;
		getaddrinfo(argv[4], argv[5], &contacthintss, &contactservinfoo);
		for(contactpp = contactservinfoo; contactpp != NULL; contactpp = contactpp->ai_next) {
			if ((sockfiled[0] = socket(contactpp->ai_family, contactpp->ai_socktype,
				contactpp->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}
			setsockopt(sockfiled[0],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
			if (connect(sockfiled[0], contactpp->ai_addr, contactpp->ai_addrlen) == -1) {
				close(sockfiled[0]);
				perror("client: connect");
				continue;
			}
			break;
		}
		char joining[14];
		joining[0] = -64;
		for(int j = 1; j < 6; j++){
			joining[j] = 0;
		}
		joining[6] = self_id >> 8;
		joining[7] = self_id & 0xFF;
		
		char localhost[10] = "localhost";
								
		if (strcmp(argv[1], localhost) == 0){
			joining[8] = 127;
			joining[9] = 0;
			joining[10] = 0;
			joining[11] = 1;
		}
		else {
			parseIPV4stringmyip(argv[1]);
			joining[8] = myip[0];
			joining[9] = myip[1];
			joining[10] = myip[2];
			joining[11] = myip[3];
		}	
		//printf("here is something: %d\n", (int)joining[8]);
		joining[12] = self_port >> 8;
		joining[13] = self_port & 0xFF;	
		send(sockfiled[0], joining, 14, 0);
		close(sockfiled[0]);
	}
	
	//int counter = 0;
	
    // add the listener to the master set
    FD_SET(listener, &master);
    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
    // main loop
	int counter = 0;
	
	suc_port = (char*) calloc(1, sizeof(char)*6);
	pre_port = (char*) calloc(1, sizeof(char)*6);
	
    for(;;) {
		//printf("T\n");
		struct timeval timeout;
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, &timeout) == -1) {
            perror("select");
            exit(4);
        }
		
		counter++;
		
		if (suc_id != self_id && counter > 6){
			memset(&hintss, 0, sizeof hintss);
			hintss.ai_family = AF_UNSPEC;
			hintss.ai_socktype = SOCK_STREAM;
			//printf("what\n");
			//printf("%d\n", suc_p);
			suc_port = (char*) calloc(1, sizeof(char)*6);
			sprintf(suc_port, "%u", suc_p);
			//printf("about\n");
			getaddrinfo((char*)suc_ip, suc_port, &hintss, &servinfoo);
			//printf("here\n");
			for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
				if ((sockfiled[1] = socket(pp->ai_family, pp->ai_socktype,
					pp->ai_protocol)) == -1) {
					perror("client: socket");
					continue;
				}
				setsockopt(sockfiled[1],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
				if (connect(sockfiled[1], pp->ai_addr, pp->ai_addrlen) == -1) {
					close(sockfiled[1]);
					perror("client: connect");
					continue;
				}
				break;
			}
			stabmessage[0] = -112;
			stabmessage[6] = self_id >> 8;
			stabmessage[7] = self_id & 0xFF;
			stabmessage[12] = self_port >> 8;
			stabmessage[13] = self_port & 0xFF;
			
			char localhost[10] = "localhost";
								
			if (strcmp(argv[1], localhost) == 0){
				stabmessage[8] = 127;
				stabmessage[9] = 0;
				stabmessage[10] = 0;
				stabmessage[11] = 1;
			}
			else {
				char intern[4];
				parseIPV4string(argv[1], intern);
				stabmessage[8] = intern[0];
				stabmessage[9] = intern[1];
				stabmessage[10] = intern[2];
				stabmessage[11] = intern[3];
			}
			
			send(sockfiled[1], stabmessage, 14, 0);
			close(sockfiled[1]);
			//printf("sending stabilizing message\n");
			counter = 0;
		}
		
		//sleep(0.1);
		
		/*if(recv(sockfiled[1], buf, 14, MSG_DONTWAIT) == -1){
			perror("recv");
		}
		else {
			byte = buf[0];
				
			for(int j = 7; 0 <= j; j --){
				headlineclient[j] = (byte >> j) & 0x01;
			}
			
			internal = headlineclient[7];
			join = headlineclient[6];
			notify = headlineclient[5];
			stabilize = headlineclient[4];
			ack = headlineclient[3];
			get = headlineclient[2];
			set = headlineclient[1];
			del = headlineclient[0];
			if (internal && stabilize && ack){
				unsigned char tmpid[2];
				tmpid[0] = buf[6];
				tmpid[1] = buf[7];
				if (suc_id != char2short(tmpid)){
					suc_id = char2short(tmpid);
				}
				printf("I got a stabilize answer from peer with ID = %d.\n", char2short(tmpid));
			}
		}*/							
		
		
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
                        perror("accept");
                    } 
					else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                    }
                } 
				else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            //printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } 
					else {
                        // we got some data from a client, so lets parse it:
						printf("I've got a message, it is ");
						byte = buf[0];
							
						for(int j = 7; 0 <= j; j --){
							headlineclient[j] = (byte >> j) & 0x01;
						}
						
						internal = headlineclient[7];
						join = headlineclient[6];
						notify = headlineclient[5];
						stabilize = headlineclient[4];
						ack = headlineclient[3];
						get = headlineclient[2];
						set = headlineclient[1];
						del = headlineclient[0];
						
						if (internal && !join && !notify && !stabilize){				//internal message. has to be parsed differently
							
							int tIDr = (int) buf[1];
							
							unsigned char klr[2];
							kli = 0;
							
							klr[0] = buf[2];
							klr[1] = buf[3];
							
							kli = char2short(klr);
							
							vli = 0;
							unsigned char vlr[2];
							
							vlr[0] = buf[4];
							vlr[1] = buf[5];
							
							vli = char2short(vlr);
							
							char hpip[4];
							for (int j = 0; j < 4; j++){
								hpip[j] = buf[j+8];
							}	

							ipchar_to_string(hpip);
							
							headpeer_p = 0;
							unsigned char hpp[2];
							
							hpp[0] = buf[12];
							hpp[1] = buf[13];
							
							headpeer_p = char2short(hpp);
							headpeer_port = (char*) calloc(1, sizeof(char)*6);
							sprintf(headpeer_port, "%u", headpeer_p);
							
							keyc = (char*) calloc(1, sizeof(char)*(kli+1));
							int pause = 0;
							
							for (int j = 14; j < 14+kli; j++){
								keyc[j-14] = buf[j];
								pause = j;
							}

							pause++;
							
							valuec = (char*) calloc(1, sizeof(char)*(vli+1));
							
							if (set == 1){
								for (int j = pause; j < pause+vli; j++){
									valuec[j-pause] = buf[j];
								}
							}
							unsigned char tmpid[2];
							tmpid[0] = buf[6];
							tmpid[1] = buf[7];
							headid = char2short(tmpid);
							printf("an internal hash message from peer with ID = %d.\n", headid);
							printf("The transactionID of this request is: %d\n", tIDr);
						}
						else if (internal && join){
							//char byte = buf[0];
							unsigned char tmpid[2];
							tmpid[0] = buf[6];
							tmpid[1] = buf[7];
							requestID = char2short(tmpid);
							if (between(pre_id, self_id, requestID) || pre_id == self_id){
								char hpip[4];
								for (int j = 0; j < 4; j++){
									hpip[j] = buf[j+8];
								}	
	
								ipchar_to_string(hpip);
								
								headpeer_p = 0;
								unsigned char hpp[2];
								
								hpp[0] = buf[12];
								hpp[1] = buf[13];
								
								headpeer_p = char2short(hpp);
								headpeer_port = (char*) calloc(1, sizeof(char)*6);
								sprintf(headpeer_port, "%u", headpeer_p);
								
								for (int j = 0; j < 12; j++){
									pre_ip[j] = 0;
								}	
								
								pre_p = headpeer_p;
								for (int j = 0; j < 12; j++){
									pre_ip[j] = headpeer_ip[j];
								}
								preipset = 1;
								sprintf(pre_port, "%u", pre_p);
								
								if (suc_id == self_id){
									suc_id = requestID;
									suc_p = headpeer_p;
									pre_p = headpeer_p;
									for (int j = 0; j < 12; j++){
										suc_ip[j] = headpeer_ip[j];
										pre_ip[j] = headpeer_ip[j];
									}
									sprintf(suc_port, "%u", suc_p);
									sprintf(pre_port, "%u", pre_p);
								}	
								printf("a request to join, the ip is %s, the port is %s and the ID is %d.\n", headpeer_ip, headpeer_port, requestID);
							}
							else {
								printf("a request to join, but it's not my department.\n");
							}	
						}	
						else if (internal && notify){
							char hpip[4];
							for (int j = 0; j < 4; j++){
								hpip[j] = buf[j+8];
							}	

							ipchar_to_string2(hpip);
							
							//printf("_____ HPIP IS %d.%d.%d.%d______ \n", (int)hpip[0], (int)hpip[1], (int)hpip[2], (int)hpip[3]);
							
							unsigned char spp[2];
							
							unsigned char tmpid[2];
							tmpid[0] = buf[6];
							tmpid[1] = buf[7];
							
							spp[0] = buf[12];
							spp[1] = buf[13];
							
							if (!suc_p){
								suc_id = char2short(tmpid);
								suc_p = char2short(spp);
								suc_port = (char*) calloc(1, sizeof(char)*6);
								sprintf(suc_port, "%u", suc_p);
							}	
							else if (char2short(tmpid) != self_id){
								suc_id = char2short(tmpid);
								suc_p = char2short(spp);
								suc_port = (char*) calloc(1, sizeof(char)*6);
								sprintf(suc_port, "%u", suc_p);
							}	
							printf("a notification from peer with ip %s, port %s.\n", suc_ip, suc_port);
						}	
						else if (internal && stabilize){
							
							char hpip[4];
							for (int j = 0; j < 4; j++){
								hpip[j] = buf[j+8];
							}	

							ipchar_to_string(hpip);
							
							headpeer_p = 0;
							unsigned char hpp[2];
							
							unsigned char tmpid[2];
							tmpid[0] = buf[6];
							tmpid[1] = buf[7];
							requestID = char2short(tmpid);
							
							hpp[0] = buf[12];
							hpp[1] = buf[13];
							
							headpeer_p = char2short(hpp);
							headpeer_port = (char*) calloc(1, sizeof(char)*6);
							sprintf(headpeer_port, "%u", headpeer_p);
							
							//stabilizeresponse = i;
							//fcntl(stabilizeresponse, F_SETFL, O_NONBLOCK);
							if (pre_id == self_id || between(pre_id, self_id, char2short(tmpid))){
								pre_id = char2short(tmpid);
								pre_p = char2short(hpp);
								pre_port = (char*) calloc(1, sizeof(char)*6);
								sprintf(pre_port, "%u", pre_p);
							}
							//printf("received bufsize: %d from socketfd %d.\n", (int)buf[0], i);
							printf("a stabilize request from peer with ID = %d.\n", char2short(tmpid));
						}
						else {
							printf("a message from a client.\n");
							clientfd = i;					// save fd of connected client
							
							int tIDr = (int) buf[1];
							printf("The transactionID of this request is: %d\n", tIDr);
							
							unsigned char klr[2];
							kli = 0;
							
							klr[0] = buf[2];
							klr[1] = buf[3];
							
							kli = char2short(klr);
							
							keyc = (char*) calloc(1, sizeof(char)*(kli+1));
							int pause = 0;
							
							for (int j = 6; j < 6+kli; j++){
								keyc[j-6] = buf[j];
								pause = j;
							}

							pause++;
							
							vli = 0;
							
							if (set == 1){
								unsigned char vlr[2];
							
								vlr[0] = buf[4];
								vlr[1] = buf[5];
							
								vli = char2short(vlr);
							}	
							valuec = (char*) calloc(1, sizeof(char)*(vli+1));
							
							if (set == 1){
								for (int j = pause; j < pause+vli; j++){
									valuec[j-pause] = buf[j];
								}
							}	
						}	
						//printf("what?!\n");
						if (get || set || del){
							idkey = atoi(keyc) % 256;
						}	
						
						/////////////////////////////////////////////////////////////// ANSWER ///////////////////////////////////////////////////////////////
						//printf("we\n");
						if (internal && join){ //join request
							if (between(pre_id, self_id, requestID) || pre_id == self_id){ // my department, I have to let him join
								pre_id = requestID;
								memset(&hintss, 0, sizeof hintss);
								hintss.ai_family = AF_UNSPEC;
								hintss.ai_socktype = SOCK_STREAM;
								//printf("are\n");
								getaddrinfo((char*)headpeer_ip, headpeer_port, &hintss, &servinfoo);
								//printf("here\n");
								for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
									if ((sockfiled[2] = socket(pp->ai_family, pp->ai_socktype,
										pp->ai_protocol)) == -1) {
										perror("client: socket");
										continue;
									}
									setsockopt(sockfiled[2],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
									if (connect(sockfiled[2], pp->ai_addr, pp->ai_addrlen) == -1) {
										close(sockfiled[2]);
										perror("client: connect");
										continue;
									}
									break;
								}
								buf[0] -= 32;  // turn join message into notify message
								buf[6] = self_id >> 8;
								buf[7] = self_id & 0xFF;
								
								char localhost[10] = "localhost";
								
								if (strcmp(argv[1], localhost) == 0){
									buf[8] = 127;
									buf[9] = 0;
									buf[10] = 0;
									buf[11] = 1;
								}
								else {
									char intern[4];
									parseIPV4string(argv[1], intern);
									buf[8] = intern[0];
									buf[9] = intern[1];
									buf[10] = intern[2];
									buf[11] = intern[3];
								}	
								
								buf[12] = self_port >> 8;
								buf[13] = self_port & 0xFF;
								
								send(sockfiled[2], buf, 14, 0);
								close(sockfiled[2]);
								printf("Joining the requesting peer with ID = %d, his successor is %d.\n", requestID, self_id);
							}	
							else { // send that to next peer
								memset(&hintss, 0, sizeof hintss);
								hintss.ai_family = AF_UNSPEC;
								hintss.ai_socktype = SOCK_STREAM;
								sprintf(suc_port, "%u", suc_p);
								getaddrinfo((char*)suc_ip, suc_port, &hintss, &servinfoo);
								for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
									if ((sockfiled[3] = socket(pp->ai_family, pp->ai_socktype,
										pp->ai_protocol)) == -1) {
										perror("client: socket");
										continue;
									}
									setsockopt(sockfiled[3],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
									if (connect(sockfiled[3], pp->ai_addr, pp->ai_addrlen) == -1) {
										close(sockfiled[3]);
										perror("client: connect");
										continue;
									}
									break;
								}
								send(sockfiled[3], buf, 14, 0);
								close(sockfiled[3]);
							}	
						}
						else if (internal && stabilize){
							
							memset(&hintss, 0, sizeof hintss);
							hintss.ai_family = AF_UNSPEC;
							hintss.ai_socktype = SOCK_STREAM;
							getaddrinfo((char*)headpeer_ip, headpeer_port, &hintss, &servinfoo);
							for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
								if ((sockfiled[7] = socket(pp->ai_family, pp->ai_socktype,
									pp->ai_protocol)) == -1) {
									perror("client: socket");
									continue;
								}
								setsockopt(sockfiled[7],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
								if (connect(sockfiled[7], pp->ai_addr, pp->ai_addrlen) == -1) {
									close(sockfiled[7]);
									perror("client: connect");
									continue;
								}
								break;
							}
							
							buf[6] = pre_id >> 8;
							buf[7] = pre_id & 0xFF;
							buf[0] += 16;
							
							//char localhost[10] = "localhost";
							
							char intern[4];
							for (int j = 0; j < 4 ; j++){
								intern[j] = 0;
							}	
							
							if (!preipset){
								snprintf(pre_ip, 12, "%d.%d.%d.%d", 127, 0, 0, 1); 
							}	
							
							parseIPV4string(pre_ip, intern);
							/*printf("______ THE PRE_IP IS %s_______\n", pre_ip);
							printf("buf[8] = %d\n", (int)intern[0]);
							printf("buf[9] = %d\n", (int)intern[1]);
							printf("buf[10] = %d\n", (int)intern[2]);
							printf("buf[11] = %d\n", (int)intern[3]);*/
							buf[8] = intern[0];
							buf[9] = intern[1];
							buf[10] = intern[2];
							buf[11] = intern[3];
							
							buf[12] = pre_p >> 8;
							buf[13] = pre_p & 0xFF;
							
							//buf[0] += 8;
							//printf("sizeof buf: %d, socketfd to send %d.\n", (int)buf[0], stabilizeresponse);
							//sleep(0.2);
							if (send(sockfiled[7], buf, 14, 0) == -1){
								perror("send");
							}	
							close(sockfiled[7]);
							//printf("Sending back notify as stabilize answer, telling that my predecessor has ID = %d.\n", pre_id);
						}	
						else if (internal && notify){
							//printf("AAAAAAAAAAAAAAAAAAAAHHHHHHHHHHHHHHHHH!!\n");
						}	
						else if ((idkey <= self_id && idkey > pre_id) || (pre_id > self_id && (idkey > pre_id || idkey <= self_id)) || pre_id == self_id){	// this is my department
							printf("The message is for me, I'm on it...\n");
							buf[0] += 8;
							if (internal){							// message has already been forwarded, so send it back to address in header
								buf[6] = self_id >> 8;
								buf[7] = self_id & 0xFF;
								normalresp = (char*) calloc(1, sizeof(char)*(14+kli+vli));
								memset(&hintss, 0, sizeof hintss);
								hintss.ai_family = AF_UNSPEC;
								hintss.ai_socktype = SOCK_STREAM;
								getaddrinfo((char*)headpeer_ip, headpeer_port, &hintss, &servinfoo);
								for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
									if ((sockfiled[4] = socket(pp->ai_family, pp->ai_socktype,
										pp->ai_protocol)) == -1) {
										perror("client: socket");
										continue;
									}
									setsockopt(sockfiled[4],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
									if (connect(sockfiled[4], pp->ai_addr, pp->ai_addrlen) == -1) {
										close(sockfiled[4]);
										perror("client: connect");
										continue;
									}
									break;
								}
								
								char localhost[10] = "localhost";
								//printf("hi\n");
								if (strcmp(argv[1], localhost) == 0){
									buf[8] = 127;
									buf[9] = 0;
									buf[10] = 0;
									buf[11] = 1;
								}
								else {
									char intern[4];
									parseIPV4string(argv[1], intern);
									buf[8] = intern[0];
									buf[9] = intern[1];
									buf[10] = intern[2];
									buf[11] = intern[3];
								}	
								//printf("hello\n");
								buf[12] = self_port >> 8;
								buf[13] = self_port & 0xFF;
								
								for (int j = 0; j < 14+kli; j++){
									normalresp[j] = buf[j];
								}
								
								if (get == 1){
									if (find_user(keyc) != NULL){
										
										unsigned short valuel = strlen(find_user(keyc)->value);
										
										printf("Getting value: %s\n", find_user(keyc)->value);
										char* getresp = (char*) calloc(1, sizeof(char)*(2+14+kli+strlen(find_user(keyc)->value)));
										
										for (int j = 0; j < 14+kli; j++){
											getresp[j] = buf[j];
										}
										
										getresp[4] = valuel >> 8;
										getresp[5] = valuel & 0xFF;
						
										for (int j = 14+kli; j < 14+kli+strlen(find_user(keyc)->value); j++){
											getresp[j] = (find_user(keyc)->value)[j-14-kli];
										}
										
										if (send(sockfiled[4], getresp, 14+kli+strlen(find_user(keyc)->value), 0) == -1){
											perror("send");
										}	
										
										free(getresp);
									}
									else {
										printf("Video not found.\n");
										normalresp[0] -= 4;
										if (send(sockfiled[4], normalresp, 14+kli+vli, 0) == -1){
											perror("send");
										}
									}	
								}
								else if (set == 1){
									for (int j = 14; j < 14+kli+vli; j++){
										normalresp[j] = buf[j];
									}	
									if (find_user(keyc) != NULL){
										delete_user(find_user(keyc));
									}
									add_user(new_video(keyc, valuec));
									printf("Video modified or added.\n");
									if (send(sockfiled[4], normalresp, 14+kli+vli, 0) == -1){
										perror("send");
									}	
								}
								else if (del == 1){
									for (int j = 14; j < 14+kli+vli; j++){
										normalresp[j] = buf[j];
									}
									if (find_user(keyc) != NULL){
										delete_user(find_user(keyc));
										printf("Video deleted.\n");
										if (send(sockfiled[4], normalresp, 14+kli+vli, 0) == -1){
											perror("send");
										}
									}
									else {
										printf("Could not delete.\n");
										normalresp[0] -= 1;
										if (send(sockfiled[4], normalresp, 14+kli+vli, 0) == -1){
											perror("send");
										}
									}		
								}	
								printf("Okay I'm done, sending answer to communication peer.\n");
								close(sockfiled[4]);
								free(normalresp);
							}
							else {
								printf("Sending answer directly back to client.\n");
								normalresp = (char*) calloc(1, sizeof(char)*7);
							
								for (int j = 0; j < 6; j++){
									normalresp[j] = buf[j];
								}
								if (get == 1){
									if (find_user(keyc) != NULL){
										
										unsigned short valuel = strlen(find_user(keyc)->value);
										
										printf("Getting value: %s\n", find_user(keyc)->value);
										char* getresp = (char*) calloc(1, sizeof(char)*(2+6+kli+strlen(find_user(keyc)->value)));
										
										for (int j = 0; j < 6+kli; j++){
											getresp[j] = buf[j];
										}
										
										getresp[4] = valuel >> 8;
										getresp[5] = valuel & 0xFF;
						
										for (int j = 6+kli; j < 6+kli+strlen(find_user(keyc)->value); j++){
											getresp[j] = (find_user(keyc)->value)[j-6-kli];
										}
										
										if (send(i, getresp, 6+kli+strlen(find_user(keyc)->value), 0) == -1){
											perror("send");
										}	
										
										free(getresp);
									}
									else {
										printf("Video not found.\n");
										normalresp[0] -= 4;
										if (send(i, normalresp, 6, 0) == -1){
											perror("send");
										}
									}	
								}
								else if (set == 1){
									if (find_user(keyc) != NULL){
										delete_user(find_user(keyc));
									}
									add_user(new_video(keyc, valuec));
									printf("Video modified or added.\n");
									if (send(i, normalresp, 6, 0) == -1){
										perror("send");
									}	
								}
								else if (del == 1){
									if (find_user(keyc) != NULL){
										delete_user(find_user(keyc));
										printf("Video deleted.\n");
										if (send(i, normalresp, 6, 0) == -1){
											perror("send");
										}
									}
									else {
										printf("Could not delete.\n");
										normalresp[0] -= 1;
										if (send(i, normalresp, 6, 0) == -1){
											perror("send");
										}
									}		
								}	
								free(normalresp);
							}	
						}
						else {										// give this request to some other peer
							if (internal){							// check whether this message has already been forwarded
								// send to next peer without altering message
								if (ack){		// complete request, send back to client
									idkey = atoi(keyc) % 256;
									unsigned char tmpidd[2];
									tmpidd[0] = buf[6];
									tmpidd[1] = buf[7];
									printf("This has been finished by peer with ID = %d, giving answer to client.\n", char2short(tmpidd));
									buf[6] = self_id >> 8;
									buf[7] = self_id & 0xFF;
									char sendclient[10000];
									sendclient[0] = buf[0] - 128;
									for (int j = 1; j < 6; j++){
										sendclient[j] = buf[j];
									}
									for (int j = 14; j < 14+kli+vli; j++){
										sendclient[j-8] = buf[j];
									}	
									
									for(int j = 0; between(self_id, suc_id, (power(2,j)+self_id)%256); j++){
										updateft((power(2,j)+self_id)%256, suc_id, suc_p, suc_ip);
										//printf("updating1: start: %d \t successor: %d\n", (power(2,j)+self_id)%256, suc_id);
									}	
									
									/*for (int j = 0; between(idkey, headid, (power(2,j)+self_id)%256); j++){
										updateft((power(2,j)+self_id)%256, headid, headpeer_p, headpeer_ip);
									}	*/
									
									int update = 0;
									
									while (update < 8){
										if (between(idkey, headid, (power(2,update)+self_id)%256)){
											updateft((power(2,update)+self_id)%256, headid, headpeer_p, headpeer_ip);
											//printf("updating2: start: %d \t successor: %d\n", (power(2,update)+self_id)%256, headid);
										}
										if (idkey == (power(2,update)+self_id)%256){
											updateft((power(2,update)+self_id)%256, headid, headpeer_p, headpeer_ip);
											//printf("updating3: start: %d \t successor: %d\n", (power(2,update)+self_id)%256, headid);
										}
										update++;
									}	
									
									send(clientfd, sendclient, 14+kli+vli, 0);
								}
								else {
									buf[6] = self_id >> 8;
									buf[7] = self_id & 0xFF;
									
									int ft = 0;
								
									for (ft = 0; between(self_id, idkey, (power(2,ft)+self_id)%256) && ft < 8; ft++){
									}
									ft--;
									
									//printf("here i come, ft is %d, corresponding port is %d\n", ft, lookupft((power(2,ft)+self_id)%256)->port);
									
									ft_port = (char*) calloc(1, sizeof(char)*6);
									sprintf(ft_port, "%u", lookupft((power(2,ft)+self_id)%256)->port);
									
									//printf("wheres the problem\n");
	
									if (!lookupft((power(2,ft)+self_id)%256)->port){
										printf("Found no fingertable entry, thus sending to next peer with ID = %d, because its not my responsibility.\n", suc_id);
										memset(&hintss, 0, sizeof hintss);
										hintss.ai_family = AF_UNSPEC;
										hintss.ai_socktype = SOCK_STREAM;
										sprintf(suc_port, "%u", suc_p);
										getaddrinfo((char*)suc_ip, suc_port, &hintss, &servinfoo);
										for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
											if ((sockfiled[5] = socket(pp->ai_family, pp->ai_socktype,
												pp->ai_protocol)) == -1) {
												perror("client: socket");
												continue;
											}
											setsockopt(sockfiled[5],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
											if (connect(sockfiled[5], pp->ai_addr, pp->ai_addrlen) == -1) {
												close(sockfiled[5]);
												perror("client: connect");
												continue;
											}
											break;
										}
									}
									else {
										printf("Found a fingertable entry, ");
										//printf("the port is %s\n", ft_port);
										memset(&hintss, 0, sizeof hintss);
										hintss.ai_family = AF_UNSPEC;
										hintss.ai_socktype = SOCK_STREAM;
										//sprintf(suc_port, "%u", suc_p);
										getaddrinfo((char*)lookupft((power(2,ft)+self_id)%256)->ip, ft_port, &hintss, &servinfoo);
										//printf("do we get here\n");
										for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
											if ((sockfiled[5] = socket(pp->ai_family, pp->ai_socktype,
												pp->ai_protocol)) == -1) {
												perror("client: socket");
												continue;
											}
											setsockopt(sockfiled[5],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
											if (connect(sockfiled[5], pp->ai_addr, pp->ai_addrlen) == -1) {
												close(sockfiled[5]);
												perror("client: connect");
												continue;
											}
											break;
										}
										printf("thus sending directly to peer with ID = %d.\n", lookupft((power(2,ft)+self_id)%256)->successor);
									}	
									
									printft();
									
									send(sockfiled[5], buf, 14+kli+vli, 0);
									close(sockfiled[5]);
								}	
							}
							else {
								char internalmsg [10000];
								internalmsg[0] = buf[0] + 128;			// add internal note
								for (int j = 1; j < 6; j++){
									internalmsg[j] = buf[j];
								}	
								internalmsg[6] = self_id >> 8;
								internalmsg[7] = self_id & 0xFF;
								
								char localhost[10] = "localhost";
								
								if (strcmp(argv[1], localhost) == 0){
									internalmsg[8] = 127;
									internalmsg[9] = 0;
									internalmsg[10] = 0;
									internalmsg[11] = 1;
								}
								else {
									char intern[4];
									parseIPV4string(argv[1], intern);
									internalmsg[8] = intern[0];
									internalmsg[9] = intern[1];
									internalmsg[10] = intern[2];
									internalmsg[11] = intern[3];
								}	
								
								internalmsg[12] = self_port >> 8;
								internalmsg[13] = self_port & 0xFF;
								
								for (int j = 14; j < 14+kli+vli; j++){
									internalmsg[j] = buf[j-8];
								}
								
								int ft = 0;
								
								for (ft = 0; between(self_id, idkey, (power(2,ft)+self_id)%256) && ft < 8; ft++){
								}
								ft--;
								
								//printf("here i come, ft is %d, corresponding port is %d\n", ft, lookupft((power(2,ft)+self_id)%256)->port);
								
								ft_port = (char*) calloc(1, sizeof(char)*6);
								sprintf(ft_port, "%u", lookupft((power(2,ft)+self_id)%256)->port);
								
								//printf("wheres the problem\n");

								if (!lookupft((power(2,ft)+self_id)%256)->port){
									printf("Found no fingertable entry, thus sending to next peer with ID = %d, because its not my responsibility.\n", suc_id);
									memset(&hintss, 0, sizeof hintss);
									hintss.ai_family = AF_UNSPEC;
									hintss.ai_socktype = SOCK_STREAM;
									sprintf(suc_port, "%u", suc_p);
									getaddrinfo((char*)suc_ip, suc_port, &hintss, &servinfoo);
									for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
										if ((sockfiled[6] = socket(pp->ai_family, pp->ai_socktype,
											pp->ai_protocol)) == -1) {
											perror("client: socket");
											continue;
										}
										setsockopt(sockfiled[6],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
										if (connect(sockfiled[6], pp->ai_addr, pp->ai_addrlen) == -1) {
											close(sockfiled[6]);
											perror("client: connect");
											continue;
										}
										break;
									}
								}
								else {
									printf("Found a fingertable entry, ");
									//printf("the port is %s\n", ft_port);
									memset(&hintss, 0, sizeof hintss);
									hintss.ai_family = AF_UNSPEC;
									hintss.ai_socktype = SOCK_STREAM;
									//sprintf(suc_port, "%u", suc_p);
									getaddrinfo((char*)lookupft((power(2,ft)+self_id)%256)->ip, ft_port, &hintss, &servinfoo);
									//printf("do we get here\n");
									for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
										if ((sockfiled[6] = socket(pp->ai_family, pp->ai_socktype,
											pp->ai_protocol)) == -1) {
											perror("client: socket");
											continue;
										}
										setsockopt(sockfiled[6],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
										if (connect(sockfiled[6], pp->ai_addr, pp->ai_addrlen) == -1) {
											close(sockfiled[6]);
											perror("client: connect");
											continue;
										}
										break;
									}
									printf("thus sending directly to peer with ID = %d.\n", lookupft((power(2,ft)+self_id)%256)->successor);
								}	
								
								printft();
								
								//fcntl(sockfiled[6], F_SETFL, O_NONBLOCK);
								send(sockfiled[6], internalmsg, 14+kli+vli, 0);
								close(sockfiled[6]);
							}
						}	
						printf("Current predecessor: %d with port %d\t\t Current successor: %d with port %d\n", pre_id, pre_p, suc_id, suc_p);
						printf("\n");
						/*
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }*/
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}


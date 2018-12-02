#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "uthash.h"

char* result;
char* keyc;
char* valuec;
char* normalresp;
char headpeer_ip[12];
char* headpeer_port;

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
    snprintf(headpeer_ip, 12, "%d.%d.%d.%d", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]); 
}

void parseIPV4string(char* ipAddress, char ipbytes[4]) {
	int tmp[4];
	sscanf(ipAddress, "%d.%d.%d.%d", &tmp[3], &tmp[2], &tmp[1], &tmp[0]);
	for(int k = 0; k < 4; k++){
		ipbytes[k] = (char) tmp[k];
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

int main(int argc, char *argv[])
{
	if (argc != 10) {
        fprintf(stderr,"usage: peer id ip port id_pre ip_pre port_pre id_suc ip_suc port_suc\n");
        exit(1);
    }
	printf("\n");
	printf("Hi, my peer ID is %s, how may I help you?\n", argv[1]);
	
	unsigned short self_id = atoi(argv[1]);
	unsigned short pre_id = atoi(argv[4]);
	unsigned short self_port = atoi(argv[3]);
	
    fd_set master;
    fd_set read_fds;
    int fdmax;
	unsigned short kli, vli, headpeer_p;
	short get, set, del, ack;
	int clientfd;

    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    char buf[10000];
    int nbytes;

    int yes=1;
    int i, j, rv;

	int sockfdd;
	struct addrinfo hintss, *servinfoo, *pp;
	struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, argv[3], &hints, &ai)) != 0) {
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

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

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
						if ((unsigned char)buf[0] >= 128){				//internal message. has to be parsed differently
							char byte = buf[0];
							int headlineclient[8];
							
							for(int j = 7; 0 <= j; j --){
								headlineclient[j] = (byte >> j) & 0x01;
							}
							
							ack = headlineclient[3];
							get = headlineclient[2];
							set = headlineclient[1];
							del = headlineclient[0];
							
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
							printf("an internal message from peer with ID = %d.\n", char2short(tmpid));
							printf("The transactionID of this request is: %d\n", tIDr);
						}
						else {
							printf("a message from a client.\n");
							char byte = buf[0]; 
							int headlineclient[8];
							clientfd = i;					// save fd of connected client
							for(int j = 7; 0 <= j; j --){
								headlineclient[j] = (byte >> j) & 0x01;
							}
							
							ack = headlineclient[3];
							get = headlineclient[2];
							set = headlineclient[1];
							del = headlineclient[0];
							
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
						
						int idkey = atoi(keyc) % 255;
						
						if ((idkey <= self_id && idkey > pre_id) || (pre_id > self_id && (idkey > pre_id || idkey <= self_id))){	// this is my department
							printf("The message is for me, I'm on it...\n");
							buf[0] += 8;
							if ((unsigned char)buf[0] >= 128){							// message has already been forwarded, so send it back to address in header
								buf[6] = self_id >> 8;
								buf[7] = self_id & 0xFF;
								normalresp = (char*) calloc(1, sizeof(char)*(14+kli+vli));
								for (int j = 0; j < 14+kli; j++){
									normalresp[j] = buf[j];
								}
								memset(&hintss, 0, sizeof hintss);
								hintss.ai_family = AF_UNSPEC;
								hintss.ai_socktype = SOCK_STREAM;
								getaddrinfo((char*)headpeer_ip, headpeer_port, &hintss, &servinfoo);
								for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
									if ((sockfdd = socket(pp->ai_family, pp->ai_socktype,
										pp->ai_protocol)) == -1) {
										perror("client: socket");
										continue;
									}
									if (connect(sockfdd, pp->ai_addr, pp->ai_addrlen) == -1) {
										close(sockfdd);
										perror("client: connect");
										continue;
									}
									break;
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
										
										if (send(sockfdd, getresp, 14+kli+strlen(find_user(keyc)->value), 0) == -1){
											perror("send");
										}	
										
										free(getresp);
									}
									else {
										printf("Video not found.\n");
										normalresp[0] -= 4;
										if (send(sockfdd, normalresp, 14+kli+vli, 0) == -1){
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
									if (send(sockfdd, normalresp, 14+kli+vli, 0) == -1){
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
										if (send(sockfdd, normalresp, 14+kli+vli, 0) == -1){
											perror("send");
										}
									}
									else {
										printf("Could not delete.\n");
										normalresp[0] -= 1;
										if (send(sockfdd, normalresp, 14+kli+vli, 0) == -1){
											perror("send");
										}
									}		
								}	
								printf("Okay I'm done, sending answer to communication peer.\n");
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
							if ((unsigned char)buf[0] >= 128){							// check whether this message has already been forwarded
								// send to next peer without altering message
								if (ack){		// complete request, send back to client
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
									send(clientfd, sendclient, 14+kli+vli, 0);
								}
								else {
									buf[6] = self_id >> 8;
									buf[7] = self_id & 0xFF;
									memset(&hintss, 0, sizeof hintss);
									hintss.ai_family = AF_UNSPEC;
									hintss.ai_socktype = SOCK_STREAM;
									getaddrinfo(argv[8], argv[9], &hintss, &servinfoo);
									for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
										if ((sockfdd = socket(pp->ai_family, pp->ai_socktype,
											pp->ai_protocol)) == -1) {
											perror("client: socket");
											continue;
										}
										if (connect(sockfdd, pp->ai_addr, pp->ai_addrlen) == -1) {
											close(sockfdd);
											perror("client: connect");
											continue;
										}
										break;
									}
									send(sockfdd, buf, 14+kli+vli, 0);
									printf("Sending to next peer with ID = %s, because its not my responsibility.\n", argv[7]);
								}	
							}
							else {
								printf("Sending to next peer with ID = %s, because its not my responsibility.\n", argv[7]);
								char internalmsg [10000];
								internalmsg[0] = buf[0] + 128;			// add internal note
								for (int j = 1; j < 6; j++){
									internalmsg[j] = buf[j];
								}	
								internalmsg[6] = self_id >> 8;
								internalmsg[7] = self_id & 0xFF;
								
								char localhost[10] = "localhost";
								
								if (strcmp(argv[2], localhost) == 0){
									internalmsg[8] = 127;
									internalmsg[9] = 0;
									internalmsg[10] = 0;
									internalmsg[11] = 1;
								}
								else {
									char intern[4];
									parseIPV4string(argv[2], intern);
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
								
								memset(&hintss, 0, sizeof hintss);
								hintss.ai_family = AF_UNSPEC;
								hintss.ai_socktype = SOCK_STREAM;
								getaddrinfo(argv[8], argv[9], &hintss, &servinfoo);
								for(pp = servinfoo; pp != NULL; pp = pp->ai_next) {
								    if ((sockfdd = socket(pp->ai_family, pp->ai_socktype,
										pp->ai_protocol)) == -1) {
										perror("client: socket");
										continue;
									}
									if (connect(sockfdd, pp->ai_addr, pp->ai_addrlen) == -1) {
										close(sockfdd);
										perror("client: connect");
										continue;
									}
									break;
								}
								send(sockfdd, internalmsg, 14+kli+vli, 0);
							}
						}	
						printf("\n");
						
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
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> //definitions of a number of data types used in system calls
#include <sys/socket.h> //socket(), connect(), send(), recv()
#include <arpa/inet.h> //sockaddr_in, inet_addr()
#include <unistd.h> //close()
#include <string.h> //memset()
#include <netinet/in.h> //header file: constants and structures needed for internet domain addresses (sockadrr_in, inet_ntoa())
#include <netdb.h> //gethostbyname
#include <ctype.h> //isdigit
#define line_len 512
#define BACKLOG 10
#include <time.h>

int main(int argc, char* argv[]) {

if (argc < 2){
    perror("ERROR. Filename & portnr.\n");
    exit(EXIT_FAILURE);
} else if (argv[1] == NULL){
    perror("ERROR. Filename.\n");
    exit(EXIT_FAILURE);
} else if(argv[2] == NULL) {
    perror("ERROR. Portnr.\n");
    exit(EXIT_FAILURE);
}

//read file
FILE *phrases;

if ((phrases = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "There is no file.");
    exit(EXIT_FAILURE);
}

char ch = getc(fopen(argv[1], "r"));
int count = 0;
int c = 0;
char* buffer = calloc(line_len, sizeof(char));

while (ch != EOF) {
    if (ch == '\n') {
        count++;
    }
    ch = getc(phrases);
}

/* Create a socket. */
//Adress- bzw. Protokollfamilie, Socket-Typ, 0 fuer Standardprotokoll (hier IPPROTO_TCP)
int socket_listen = socket(PF_INET, SOCK_STREAM, 0);

if (socket_listen == -1) {
        //Wenn -1 -> Fehler beim Erzeugen des Sockets
        perror("Could not create a socket.\n");
        exit(EXIT_FAILURE);
}
    
/* Socket initialising */
struct sockaddr_in server_addr;

memset(&server_addr, 0, sizeof(server_addr)); //fill the structure with zeros
server_addr.sin_family = AF_INET; //Adressfamilie
server_addr.sin_port = htons(atoi(argv[2])); //htons(): converts a port number in host byte order to a port number in network byte order
server_addr.sin_addr.s_addr = INADDR_ANY;

int error = 0;
//lokale Adresse (IP und POrtnr) zuordnen, unter welcher der Server auf Anfragen der Clients wartet
error = bind(socket_listen, (struct sockaddr*)&server_addr, sizeof(server_addr));
if (error == -1) {
    perror("ERROR. Failed to bind socket to address.\n");
    return error;
}
error = listen(socket_listen, BACKLOG);
if (error == -1) {
    perror("ERROR. failed to put socket in passive mode.\n");
    return error;
}

/* Client address */
struct sockaddr_in client_addr;

socklen_t client_len = sizeof(client_addr);

int conn_fd = accept(socket_listen, (struct sockaddr*)&client_addr, &client_len);
if(conn_fd == -1) {
    perror("ERROR. Failed accepting connection.");
    exit(EXIT_FAILURE);
}

srand(time(0));
int random = rand() % count;
phrases = fopen(argv[1], "r");

while(fgets(buffer, line_len, phrases)) {
    if (random == c) break;
    c++;
}

send(conn_fd, buffer, strlen(buffer), 0);
close(conn_fd);

fclose(phrases);
close(socket_listen);
}









































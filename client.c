//http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
//http://openbook.rheinwerk-verlag.de/c_von_a_bis_z/025_c_netzwerkprogrammierung_004.htm#mjc55073cb49f2137a6fe9e8565b3f2df6
//https://github.com/codergs/TCP-UDP-DNS-Server-in-C/blob/master/UDP_Client.c
#include <stdlib.h>
#include <stdio.h>

//API
#include <sys/types.h> //definitions of a number of data types used in system calls
#include <sys/socket.h> //socket(), connect(), send(), recv()
#include <arpa/inet.h> //sockaddr_in, inet_addr()
#include <unistd.h> //close()
#include <string.h> //memset()
#include <netinet/in.h> //header file: constants and structures needed for internet domain addresses (sockadrr_in, inet_ntoa())
#include <netdb.h> //gethostbyname
#include <ctype.h> //isdigit


int main(int argc, char* argv[]) {
    
    if (argc < 2){
        perror("ERROR. Please type DNS or IP and a port number.\n");
        exit(1);
    }else if (argv[1] == NULL){
        perror("ERROR. Please type DNS or IP.\n");
    } else if (argv[2] == NULL) {
        perror("ERROR. Please type a port number.\n");
    }
    
    //Parsing
    char ip_dns[100] = "";
    strcpy(ip_dns, argv[1]);
    
    // Create a socket.
    //Erzeugt einen neuen Kommunikationsendpunkt.
    int client_socket;
    //Adress- bzw. Protokollfamilie, Socket-Typ, 0 fuer Standardprotokoll (hier IPPROTO_TCP)
    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    
    if (client_socket == -1){
        //Wenn -1 -> Fehler beim Erzeugen des Sockets
        perror("Could not create a socket.\n");
    }
    
    // Socket initialising
    struct sockaddr_in server_addr;
    /* A sockaddr_in is a structure containing an internet address, has four fields. The definition:
        struct sockaddr_in {
            short   sin_family;
            u_short sin_port;
            struct  in_addr sin_addr;
            char    sin_zero[8];
        };*/
    //Set the fields.
    memset(&server_addr, 0, sizeof(server_addr)); //fill the structure with zeros
    //bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_port = htons(atoi(argv[2])); //htons(): converts a port number in host byte order to a port number in network byte order
    server_addr.sin_family = AF_INET; //Adressfamilie
    
    // Get server's IP and standard service connection.
    if(!isdigit(argv[1][0])) {
        //If host is a name, get IP from domain.
        struct hostent* info = gethostbyname(ip_dns); //takes a string and returns a struct hostent which contains tons of information, including the IP address
        struct in_addr **listi; //store IP address in it
        listi = (struct in_addr**) info->h_addr_list; //h_adrr_list: vector of addresses for the host, the host might be connected to multiple networks and have different addresses on each one
        //Return the first one.
        server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*listi[0])); //inet_ntoa() converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
    } else{
        server_addr.sin_addr.s_addr = inet_addr(ip_dns); //converts the Internet host address cp from IPv4 numbers-and-dots notation into binary data in network byte order
    }
    
    // Connect to server.
    //Versucht aktiv, eine Verbindung einzurichten.
    //connect(socket file descriptor, address of the host (including the port number), size of this address)
    if(connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
        //Gibt -1 bei einem Fehler zurueck
        perror("ERROR. Could not connect server.\n");
    }
    
    char text[10000]; //buffer
    //Empfaengt Daten ueber die Verbindung.
    //recv(socket file descriptor, buffer where the message is stored, length in bytes of the buffer, type of message recv - standard)
    recv(client_socket, text, 10000, 0);
    printf("%s", text);
    
    //Gibt Socket Deskriptor bzw. die Verbindung frei, wenn mit Datenuebertragung fertig.
    close(client_socket);
} 

#include "Header.h"
#include "hashtable.h"
#include "Message.h"

#define HASHSIZE 3

int main(int argc, char** argv){

    if(argc != 2) {
    
        printf("ERROR. There is no port.");
        exit(1);
    }
    
    DataItem_i* *hashArray =erstelle(HASHSIZE);
    struct addrinfo *getinfo, *rp;
    struct addrinfo hints;
    int get_info;
    int sock_fd;
    int bindp;
    int listn;
    const int queue = 3;
    const char* portnr = argv[1]; //reade the port as a string
    
    memset(&hints, 0, sizeof(hints)); //fill the mem of hints with zeros
    hints.ai_family = AF_UNSPEC; //allows IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM; //socket type: Stream, TCP
    hints.ai_flags = AI_PASSIVE; //socket addr is intended for bind()
    hints.ai_next = NULL;
    
    if((get_info = getaddrinfo(NULL, portnr, &hints, &getinfo)) != 0) { //returns a list of address structures contains an internet address
    
        printf("ERROR. getaddrinfo");
        exit(1);
    }
    
    //try each address until we successfully bind; if socket or bind fails, we close the socket and try the next address:
    for(rp = getinfo; rp != NULL; rp = rp->ai_next) {
    
        sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); //create socket
        
        if(sock_fd == -1) {
        
            continue; //if it fails, try another address
        }
        //assign the address specified by ai_addr to the socket referred by the file descriptor sock_fd
        if((bindp = bind(sock_fd, rp->ai_addr, rp->ai_addrlen)) == 0) {
        
            break; //successfully
        }
        
        close(sock_fd);
    }
    
    if(rp == NULL) {
    
        printf("ERROR. bind() and create()");
        exit(1);
    }
    
    if((listn = listen(sock_fd, queue)) != 0) { //listen to the client connection requests
        //waiting queue = 3
    
        printf("ERROR. listen()");
        exit(1);
    }
    
    while(1) { //let server run forever
    
        int new_sock_fd = accept(sock_fd, NULL, NULL); //accept client connection
        
        Message * m = receiveMessage(new_sock_fd);
        
        DataItem_i *requested = NULL;
        
        byte transaction_id = m->header.transaction_id;
		byte iget = m->header.get;
		byte iset = m->header.set;
		byte idel = m->header.del;
        
        if(m->header.del){
        } else {
            delete(hashArray, HASHSIZE, m->key, m->header.key_length);
            if(m->header.set) {
                set(hashArray, HASHSIZE, m->key, m->value, m->header.key_length, m->header.value_length);
            }
            if(m->header.get) {
                requested = get(hashArray, HASHSIZE, m->key, m->header.key_length);
            }
        }
        
        Message *iresponse = response(requested, transaction_id, iget, iset, idel);
        sendMessage(new_sock_fd, iresponse);
        
        deleteMessage(iresponse);
        deleteMessage(m);
        close(new_sock_fd);
    }
    
    freeaddrinfo(getinfo);
}

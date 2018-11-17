#include "Message.h"
#include <errno.h>


static uint16_t sfb(byte* buf)
{
	uint16_t x;
    memcpy(&x, buf, 2);
    x = ntohs(x); //ntohs: converts the unsigned short integer from network byte order to host byte order
    return x;
}

static void stb(uint16_t x, byte* buf)
{
    //Most data transmissions across a network are sent using Network-Byte-Order, or Big Endian, format
    x = htons( x ); //converts the unsigned integer byte order to network byte order
    memcpy(buf, &x, 2);
}

Message* rhb(byte buf[hsize] ){
    //read header
    Message* m = calloc(1, sizeof(Message));
    memcpy(&(m->header), buf, hsize);
    m -> header.transaction_id = buf[1];
    m -> header.key_length = sfb(&(buf[2])); //received 2 bytes from the header as a keylen/valuelen
    m -> header.value_length = sfb(&(buf[4]));
    return m;
}

Message* response(DataItem_i* value, byte transaction_id, byte get, byte set, byte del)
{
    Message* m = calloc(1, sizeof(Message));
    m->header.ack = 1;
    m->header.get = get ? 1 : 0; //if (get=1) {1} else 0
    m->header.set = set ? 1 : 0;
    m->header.del = del ? 1 : 0;
    m->header.transaction_id = transaction_id;
    m->header.key_length = 0;
    m->header.value_length = value ? value->datalen : 0;
    m->key = NULL;
    if (m->header.value_length) {
        m->value = calloc(value->datalen, sizeof(char));
        memcpy(m->value, value->data, value->datalen);
    } else {
        m->value = NULL;
    }
    return m;
}

void sendMessage(int sockfd, Message* m)
{
    //message to buffer:
    byte* buf = calloc(hsize + m->header.key_length + m->header.value_length, sizeof(byte));
    
    memcpy(buf, &(m->header), hsize);
    buf[1] = m->header.transaction_id;
    stb(m->header.key_length, &(buf[2]));
    stb(m->header.value_length, &(buf[4]));
    
    if (m->key && m->header.key_length) {
        
        memcpy(&(buf[hsize]), m->key, m->header.key_length);
    }
    if ( m->value && m->header.value_length) {
        
        memcpy(&(buf[hsize + m->header.key_length]), m->value, m->header.value_length);
    }

    size_t buffer_size = hsize + m->header.key_length + m->header.value_length;
    
    ssize_t smb = write( sockfd, buf, buffer_size ); //write data out of the buffer -> send message buffer
    if (smb < 0) {
        
        printf("ERROR. send\n");
    }

    free(buf);
}

static void readfromsocket(int sockfd, char* buf, size_t buf_len)
{

	struct timespec t1;
    memset(&t1, 0, sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, &t1);
    
    struct timespec t2;
    memset(&t2, 0, sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, &t2);
    
    for (; (t2.tv_sec - t1.tv_sec) < 5; clock_gettime(CLOCK_REALTIME, &t2)) {
        
        ssize_t rcv = recv(sockfd, buf, buf_len, MSG_DONTWAIT); //MSG_DONTWAIT: is a non-blocking operation
        if ( rcv > 0 ) { //the header in this case is received
            return;
        } else if (errno == EAGAIN) { //Resource temporarily unavailable
            // try again
            continue;
        } else if (errno == EWOULDBLOCK) {
            // if the operation would block and the error code is different from EAGAIN, then use blocking operation and return
            recv(sockfd, buf, buf_len, 0);
            return;
        } else if (rcv < 0) {
            
            printf("ERROR. receive");
            exit(1);
        } else {
            //nothing to receive
            continue;
        }
    }
}

Message* receiveMessage(int sockfd)
{
    
    byte buf[hsize]; //to read the header, the first 6 bytes
    memset(buf, 0, sizeof(buf)); //set the buffer

    readfromsocket(sockfd, (char*) buf, hsize);
    Message* m = rhb(buf);
    
    if (m->header.key_length) {
        
        m->key = calloc(m->header.key_length + 1, sizeof(byte) );
        readfromsocket(sockfd, m->key, m->header.key_length); //read the key value from the socket
    }

    if (m->header.value_length) {
        
        m->value = calloc(m->header.value_length + 1, sizeof(byte)); //allocate for the value
        readfromsocket(sockfd, m->value, m->header.value_length); //read the value from the socket
    }
    return m;
}

void deleteMessage( Message* x )
{
    if ( x -> header.key_length ) {
        free( x -> key );
    }
    if ( x -> header.value_length ) {
        free( x -> value );
    }

    free( x );
}

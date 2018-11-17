#ifndef MESSAGE_HEADER_H_
#define MESSAGE_HEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in_systm.h>
#include <assert.h>
#include <time.h>
#define hsize 6 //header size
#define hashtablesize 16

typedef unsigned char byte;

typedef struct Header
{
	byte del : 1; // 1 bit
	byte set : 1; // 1 bit
	byte get : 1; // 1 bit
	byte ack : 1; // 1 bit
	byte reserved : 4; // 1 bits
	byte transaction_id;  // 1 byte
	uint16_t key_length; // 2
	uint16_t value_length; // 2 byte

} Header;


#endif

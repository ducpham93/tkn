#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "Header.h"
#include "hashtable.h"

/**
 * The whole message has the header, key, value
 */
typedef struct Message {
    Header header;
    char* key;
    char* value;

} Message;

Message* rhb(byte header[hsize]);
Message* response(DataItem_i* value, byte transaction_id, byte get, byte set, byte del);
void sendMessage(int sockfd, Message* m);
Message* receiveMessage(int sockfd);
void deleteMessage(Message* m);


#endif

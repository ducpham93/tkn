#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


struct DataItem 
{
     char* data;   
     char* key  ;
     uint16_t   keylen;
     uint16_t   datalen;
     int flag ;
    
};
typedef struct DataItem DataItem_i;


 DataItem_i **erstelle (int sizehash) ;

int hashCode( char * key, uint16_t len,int SIZE) ;

DataItem_i *erstelle_element(uint16_t keylen ,uint16_t datalen);

int set(struct DataItem* *hashArray,int SIZE,char * key,char * data ,uint16_t keylen ,uint16_t datalen );  
// return -1  if hash table is full 0 if successful and 1 if updated and 2 if already inside 



 DataItem_i *get(struct DataItem* *hashArray,int SIZE,char * key,uint16_t keylen) ; 
//return Null if not found



int delete(DataItem_i* *hashArray,int SIZE,char *key,int keylen) ;
//return 0 if successful otherwise -1

void freehashtabel(struct DataItem* *hashArray,int SIZE);
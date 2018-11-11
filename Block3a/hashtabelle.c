#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "hashtabelle.h"




DataItem_i **erstelle (int sizehash)
{


	DataItem_i** hashArray = (DataItem_i**)malloc(sizeof(DataItem_i * )*sizehash);
  for (int i = 0; i < sizehash; ++i)
  {
      hashArray[i]= NULL;
  }
	
	return hashArray ;
}


int hashCode( char * key, uint16_t len,int SIZE) 
{
      int hash = 5381;
 
    for (int i = 0; i < len; ++i)
    {
    	hash = ((hash << 5) + hash) +  (*key); /* hash * 33 + c */   
    	key++ ;
        
    }
      hash = hash % SIZE ;    
    return hash;
}



DataItem_i *erstelle_element(uint16_t keylen ,uint16_t datalen)
{
   DataItem_i *item = (DataItem_i*) malloc(sizeof(DataItem_i)*1);
   item->data = NULL;  
   item->key = NULL;
   item->keylen = keylen ;
   item->datalen = datalen ;
   item->flag = 0 ;
  
   return item ;

}

int set(DataItem_i* *hashArray,int SIZE,char * key,char * data ,uint16_t keylen ,uint16_t datalen )
{

   


   //get the hash 
   int hashIndex = hashCode(key,keylen,SIZE);

   //move in array until an empty or deleted cell
   DataItem_i *searching = get(hashArray,SIZE,key, keylen);
   // printf("%s\n",searching->data );
    if(searching  )
    {
       if (!strncmp(searching->data , data,searching->datalen ))
         return 2 ;
        if (searching->data )
          free(searching->data) ;
        searching->data = (char *)calloc(datalen,sizeof(char));
        memcpy(searching->data,data,datalen);
      
         return 1 ;
    }
   int i = 0 ;

   while(hashArray[hashIndex] != NULL  &&  strcmp("deleted",hashArray[hashIndex]->key) ) 
   {
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
      i++;
      if (i == SIZE)
        return -1 ;
   }
   if (hashArray[hashIndex])
   {
       if (hashArray[hashIndex]->key && strcmp(hashArray[hashIndex]->key ,"deleted"))                     // delete the old key 
       free(hashArray[hashIndex]->key) ;  
       
       if (hashArray[hashIndex]->data )                   // delete the old data 
       free(hashArray[hashIndex]->data) ;
       free(hashArray[hashIndex]);
   }

   DataItem_i *new = erstelle_element(keylen,datalen);
   new->key = (char *)calloc(keylen,sizeof(char));
   new->data = (char *)calloc(datalen,sizeof(char));
   memcpy(new->key,key,keylen);
   memcpy(new->data,data,datalen);


   
	
   hashArray[hashIndex] = new;
   return 0 ;
}




DataItem_i *get(DataItem_i* *hashArray,int SIZE,char * key,uint16_t keylen) 
{
   //get the hash 
   int hashIndex = hashCode(key,keylen,SIZE);  
	
   //move in array until an empty 
   int i = 0 ;


   while(hashArray[hashIndex] != NULL) 
   {
	   
        if(!strncmp(hashArray[hashIndex]->key , key,hashArray[hashIndex]->keylen ))
         return hashArray[hashIndex]; 
      
     
     
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
      i++;
      if(i == SIZE)
        break;
   }        
	
   return NULL;        
}



int delete(DataItem_i* *hashArray,int SIZE,char *key,int keylen) {
   

   //get the hash 
   int hashIndex = hashCode( key, keylen,SIZE);

   //move in array until an empty
   while(hashArray[hashIndex] != NULL)
    {
	
      if(!strncmp(hashArray[hashIndex]->key ,key,hashArray[hashIndex]->keylen )) // do easy delete
      {

          
          
           if (hashArray[hashIndex]->key)
             free(hashArray[hashIndex]->key);
          if (hashArray[hashIndex]->data)
            free(hashArray[hashIndex]->data);
          hashArray[hashIndex]->data = NULL ;
          hashArray[hashIndex]->key = "deleted";
          
          break;
      }
		
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
   }      
	
   return -1;        
}

void freehashtabel(DataItem_i* *hashArray,int SIZE)
{
	for (int hashIndex = 0; hashIndex < SIZE; hashIndex++)
	{	
		if (hashArray[hashIndex] != NULL )
		{
       
        if( strcmp("deleted",hashArray[hashIndex]->key) )
        {
            if (hashArray[hashIndex]->key )                     // delete the old key 
            free(hashArray[hashIndex]->key) ;  
            if (hashArray[hashIndex]->data )                   // delete the old data 
           free(hashArray[hashIndex]->data) ;
        }

        free(hashArray[hashIndex]);
     
      

			
		}
		
	}
	free(hashArray);
}

// int main(int argc, char** args)
// {
	
// 	int x =  20 ;
//     DataItem_i* *hashArray =erstelle(x);

//  	set(hashArray,x,"iam the key ","value",strlen("iam the key "),0) ;

//  	DataItem_i * new = get(hashArray,x,"iam the key ",strlen("iam the key "));
//  	printf("%s\n",new->data );

//  	delete(hashArray,x,new);
//  	printf("%d\n",new->flag );

//  	freehashtabel(hashArray,x) ;



// }
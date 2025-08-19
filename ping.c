#include "ping.h"

void error(const int8* msg){
   fprintf(stderr,RED"Error:%s\n",msg);
   exit(EXIT_FAILURE);
}
icmp *create_icmp_packet(uint8 type,uint8 code,uint8 *data,uint16 size){
     icmp *packet;

     packet=malloc(sizeof(icmp)+size);
     if(!packet){
         error("Failed to allocate memory for the ICMP");  
     }
     packet->code=code;
     packet->type=type;
     if(data && size>0) memcpy(packet->data,data,size);
     
     return packet;
}
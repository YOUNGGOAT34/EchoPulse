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

uint16 _checksum(uint16 *data,size_t n){
      uint32 sum=0;
   
      for(size_t i=0;i<n;i++){
            sum+=data[i];
      }

      while(sum>>16){
              sum=(sum>>16)+(sum & 0xFFFF);
      }

     
      return (uint16)~sum;
}
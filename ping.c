#include "ping.h"
#include "hexadump.h"

void error(const int8* msg){
   fprintf(stderr,RED"Error:%s\n",msg);
   exit(EXIT_FAILURE);
}


void print_icmp_packet(icmp *packet,uint16 size){
      printf("Type: %d\tcode :%d\tchecksum: %04x\n",packet->type,packet->code,packet->checksum);
      hexadump(packet->data,size);
}

icmp *create_icmp_packet(uint8 type,uint8 code,uint8 *data,uint16 size){
     icmp *packet;
      
     packet=malloc(sizeof(icmp)+size);
     if(!packet){
         error("Failed to allocate memory for the ICMP");  
     }
     memset(packet,0,sizeof(icmp));
     packet->code=code;
     packet->type=type;
     if(data && size>0) memcpy(packet->data,data,size);
     packet->checksum=0;
     packet->checksum=checksum(packet,size+sizeof(icmp));
     return packet;
}

 
uint16 _checksum(uint8 *data,size_t n){
      
      uint32 sum=0;
      for(size_t i=0;i<n;i+=2){
          uint16 word=data[i]<<8;
          if(i+1<n) word |=data[i+1];
          sum+=word;
          printf("sum+%x=%x\n",word,sum);
      }
      
      while(sum>>16){
              sum=(sum>>16)+(sum & 0xFFFF);
      }
       printf("%04x\n",sum);
      return (uint16)~sum;
}


uint16 checksum(icmp *packet,uint16 size){
    uint8 *buff=(uint8 *)packet;
    
    if(size % 2 !=0){
        //padding
        uint8 *padded_packet=malloc(size+1);
        if(!padded_packet){
            error("Failed to allocate memory for padded packet");
        }
        memcpy(padded_packet,buff,size);
        padded_packet[size]=0;

        
        uint16 res=_checksum(padded_packet,((sizeof(icmp)+size)));
        free(padded_packet);
        return res;
    }
     
    return _checksum(buff,sizeof(icmp)+size);    
}
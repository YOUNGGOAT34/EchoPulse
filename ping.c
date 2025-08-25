#include "ping.h"
#include "hexadump.h"

void error(const i8* msg){
   fprintf(stderr,RED"Error:%s\n",msg);
   exit(EXIT_FAILURE);
}


void print_icmp_packet(icmp *packet,u16 size){
      printf("Type: %d\tcode :%d\tchecksum: %04x\n",packet->type,packet->code,packet->checksum);
      hexadump(packet->data,size);
}

icmp *create_icmp_packet(u8 type,u8 code,u8 *data,u16 size){
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

 
u16 _checksum(u8 *data,size_t n){
      
      u32 sum=0;
      for(size_t i=0;i<n;i+=2){
          u16 word=data[i]<<8;
          if(i+1<n) word |=data[i+1];
          sum+=word;
          printf("sum+%x=%x\n",word,sum);
      }
      
      while(sum>>16){
              sum=(sum>>16)+(sum & 0xFFFF);
      }
       printf("%04x\n",sum);
      return (u16)~sum;
}


u16 checksum(icmp *packet,u16 size){
    u8 *buff=(u8 *)packet;
    
    if(size % 2 !=0){
        //padding
        u8 *padded_packet=malloc(size+1);
        if(!padded_packet){
            error("Failed to allocate memory for padded packet");
        }
        memcpy(padded_packet,buff,size);
        padded_packet[size]=0;

        
        u16 res=_checksum(padded_packet,((sizeof(icmp)+size)));
        free(padded_packet);
        return res;
    }
     
    return _checksum(buff,sizeof(icmp)+size);    
}
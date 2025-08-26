#include "icmp.h"
#include "hexadump.h"

void error(const i8* msg){
   fprintf(stderr,RED"Error:%s\n",msg);
   exit(EXIT_FAILURE);
}

u8 *create_raw_icmp(icmp *packet){
    if(!packet || !packet->data){
         return NULL;
    }

    raw_icmp rawpkt;

    switch(packet->type){
         case echo:
            rawpkt.type=8;
            rawpkt.code=0;
            break;
         case reply:
            rawpkt.type=0;
            rawpkt.code=0;
            break;

        default:
            return NULL;
    }
    rawpkt.checksum=0;
    u16 total_size=sizeof(raw_icmp)+packet->size;
    if(total_size%2){
        total_size++;
    }
    u8 *p=(u8 *)malloc(total_size);
    u8 *ptr=p;
    if(!p){
        error("Failed to allocate memory for the ICMP packet pointer\n");
    }

    memset(p,0,total_size);
    memcpy(p,(u8 *)&rawpkt,sizeof(raw_icmp));
    p+=sizeof(raw_icmp);
    memcpy(p,packet->data,packet->size);
    u16 check=_checksum(ptr,total_size);
    check=htons(check);
    memcpy(ptr+2,&check,sizeof(u16));

    return ptr;


}

void print_icmp_packet(icmp *packet,u16 size){
    //   printf("Type: %d\tcode :%d\tchecksum: %04x\n",packet->type,packet->code,packet->checksum);
      hexadump(packet->data,size);
}

icmp *create_icmp_packet(TYPE type,u8 *data,u16 size){
     icmp *packet;
      
     packet=malloc(sizeof(icmp)+size);
     if(!packet){
         error("Failed to allocate memory for the ICMP");  
     }
     memset(packet,0,sizeof(icmp));
     
     packet->type=type;
     packet->size=size;
     packet->data=data;

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
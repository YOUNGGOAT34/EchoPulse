#include "ip.h"

u32 get_local_ip(void){
    int sockfd=socket(AF_INET,SOCK_STREAM,0);

    if(sockfd<0) error("Failed to create a socket for local ip retrivin\n");

    struct sockaddr_in server;
    memset(&server,0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=inet_addr("8.8.8.8");
    server.sin_port=htons(80);

    if(connect(sockfd,(struct sockaddr *)&server,sizeof(server))<0){
        error("Failed to connect to the google dns server\n");
    }

    struct sockaddr_in local;
    socklen_t len=sizeof(local);
    if(getsockname(sockfd,(struct sockaddr *)&local,&len)==-1){
        error("Error while getting the local socket information\n");
    }

    u32 ip=local.sin_addr.s_addr;

    if(ip==INADDR_ANY || ip==INADDR_NONE){
        error("Invalid local ip: wildcard or none\n");
    }

    u32 host_ip=ntohl(ip);
    const u8 a=(host_ip>>24)&0xFF;

    if(a==127){
       error("local Ip is loopback address, not usable\n");
    }

    if(a==169 && ((host_ip>>16)&0xFF)==254){
         error("local Ip is link-local ,not usable\n");
    }

   close(sockfd);

    return local.sin_addr.s_addr;
  
}

IP *create_ip(const u8 type,u16 id,const i8 *dst){
      if(!dst){
          return NULL;
      }

      IP *packet=(IP *)malloc(sizeof(IP));
      if(!packet){
         error("Failed to allocate memory for IP packet\n");
      }

      u32 src=get_local_ip();
      u32 dst_ip=inet_addr(dst);
      
      if(dst_ip==INADDR_NONE){
         error("invalid destination ip address\n");
      }
      memset(packet,0,sizeof(IP));
       
      packet->dst=dst_ip;
      packet->id=id;
      packet->src=src;
      packet->type=type;
      packet->payload=NULL;

      return packet;

  
}

u8 *create_raw_ip(IP *packet){
      if(!packet) return NULL;
      u8 protocal=0;
      switch(packet->type){
         case ICMP:
            protocal=1;
            break;

        default:
            return NULL;
      }

      u16 ttlength=sizeof(raw_icmp)+sizeof(RAWIP);
      if(packet->payload){
        ttlength+=packet->payload->size;
      }
      
      RAWIP rawpkt;
      rawpkt.ihl=sizeof(RAWIP)/4;
      rawpkt.checksum=0;
      rawpkt.id=htons(packet->id);
      rawpkt.dscp=0;
      rawpkt.ecn=0;
      rawpkt.flags=0;
      rawpkt.fragmet_offset=0;
      rawpkt.protocal=protocal;
      rawpkt.length=htons(ttlength);
      rawpkt.src=packet->src;
      rawpkt.dst=packet->dst;
      rawpkt.TTL=250;
      rawpkt.version=4;




}


void print_ip(u32 ip){
    printf("%u.%u.%u.%u\n",
      (ip>>24) & 0xFF,
      (ip>>16) & 0xFF,
      (ip>>8) & 0xFF,
       ip & 0xFF
    );
}


void print_ip_packet(IP *packet){
     if(!packet){
         error("Null IP packet\n");
     }

     printf("Type: %02hhx\n",packet->type);
     printf("Type: %02hx\n",packet->id);
     printf("src: ");
     print_ip(packet->src);
     printf("dst: ");
     print_ip(packet->dst);

     if(packet->payload){
        print_icmp_packet(packet->payload,packet->payload->size);
     }

}
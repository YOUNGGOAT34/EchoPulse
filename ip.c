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
     

      return packet;

  
}
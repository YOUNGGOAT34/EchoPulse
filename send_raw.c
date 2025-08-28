#include "send_raw.h"

bool send_raw_ip(IP *packet){
     if(!packet){
       error("Cannot send a null packet\n");
     }
     i32 sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
     if(sockfd<0){
        error("Raw socket creation failed\n");
     }

     i32 one=1;
     if(setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&one,sizeof(one))<0){
        error("Failed to set socket options\n");
     }

     u8 *raw_ip=create_raw_ip(packet);
     if(!raw_ip){
        error("Failed to create raw ip bytes\n");
     }

     struct sockaddr_in dst;
     dst.sin_family=AF_INET;
     dst.sin_addr.s_addr=htonl(packet->dst);   
      
     size_t size=sizeof(RAWIP)+sizeof(raw_icmp);
     if(packet->payload){
         size+=packet->payload->size;
     }
     if(sendto(sockfd,raw_ip,size,0,(const struct sockaddr *)&dst,sizeof(dst))<0){
         error("sending raw ip packet\n");
     }

     
     
     return true;
}
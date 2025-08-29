#include "send_raw.h"

void send_raw_ip(IP *packet){
     if(!packet){
       error("Cannot send a null packet\n");
     }
     i32 sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
     if(sockfd<0){
        error("Raw socket creation failed\n");
     }

     struct timeval tv;
     tv.tv_sec = 4;
     tv.tv_usec = 0;
     setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

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
     dst.sin_addr.s_addr=packet->dst;   
      
     size_t size=sizeof(RAWIP)+sizeof(raw_icmp);
     if(packet->payload){
         size+=packet->payload->size;
     }

     ssize_t bytes_sent=sendto(sockfd,raw_ip,size,0,(const struct sockaddr *)&dst,sizeof(dst));
     if(bytes_sent<0){
         error("sending raw ip packet\n");
     }

  

     
     recv_ip_packet(sockfd);

     close(sockfd);


     
}


void recv_ip_packet(i32 sockfd){
  
  
 

   i8 buffer[65536];
   struct sockaddr_in src_addr;
   socklen_t addr_len=sizeof(src_addr);
   size_t buff_len=sizeof(buffer);
   ssize_t bytes_received=recvfrom(sockfd,buffer,buff_len,0,(struct sockaddr *)&src_addr,&addr_len);
   
   if(bytes_received<0){
      perror("Response Error\n");
      exit(1);
   }

   RAWIP *res=(RAWIP *)buffer;

   raw_icmp *ricmp=(raw_icmp *)(buffer+(res->ihl*4));

   if(ricmp->type==0){
      printf(GREEN "Got ICMP Echo Reply from %s\n" RESET,
         inet_ntoa(src_addr.sin_addr));
   }else if(ricmp->type==3){
      printf(RED "Destination Unreachable (code %d)\n" RESET, ricmp->code);
   }

   printf("%ld bytes ",bytes_received);
   printf("from ");
   // print_ip(packet->dst);
   

}
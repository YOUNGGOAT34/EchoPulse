#include "send_raw.h"

void send_raw_ip(IP *packet){
     struct timeval start,end;
     if(!packet){
       error("Cannot send a null packet\n");
     }
     i32 sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
     if(sockfd<0){
        error("Raw socket creation failed\n");
     }

     struct timeval tv;
     tv.tv_sec = 10;
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
      gettimeofday(&start,NULL);
     ssize_t bytes_sent=sendto(sockfd,raw_ip,size,0,(const struct sockaddr *)&dst,sizeof(dst));
     if(bytes_sent<0){
         error("sending raw ip packet\n");
     }

  

     
     recv_ip_packet(sockfd);
     gettimeofday(&end,NULL);
     long rtt=((end.tv_sec-start.tv_sec)*1000L)+((end.tv_usec-start.tv_usec)/1000L);
     printf("time=%ld ms\n",rtt);
     close(sockfd);


     
}


void recv_ip_packet(i32 sockfd){
  
  
 

   i8 buffer[65536];
   struct sockaddr_in src_addr;
   socklen_t addr_len=sizeof(src_addr);
   size_t buff_len=sizeof(buffer);
   ssize_t bytes_received=recvfrom(sockfd,buffer,buff_len,0,(struct sockaddr *)&src_addr,&addr_len);
   
   if(bytes_received<0){
      if(errno==EAGAIN || errno==EWOULDBLOCK){
         printf("Request timed out\n");
        
      }else if(errno==ECONNREFUSED){
         printf("Connection refused\n");
      }else{

         error("Response Error\n");
      }
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
   printf("from %s: ",print_ip(src_addr.sin_addr.s_addr));
   printf("icmp_seq=%hd ",ntohs(ricmp->sequence));
  


}
#include "main.h"

/*
  
  What if it can ping multiple hosts ,,and possibly ping the entire subnet???

*/


volatile sig_atomic_t keep_sending=0;

void handle_sigInt(__attribute__((unused)) i32 sig){
   keep_sending=1;
}


int main(i32 argc,i8 *argv[]){

    

     command_parser(argc,argv);

     
     signal(SIGINT,handle_sigInt);

     

     return 0;

}


void command_parser(i8 argc,i8 *argv[]){
    
     if(argc<2){
          help();
          exit(EXIT_SUCCESS);
     }
      


   static struct option long_options[]={
     {"count",required_argument,0,'c'},
     {0,0,0,0}
   };


   return;

  u8 *data = (u8 *)"Hello";
     u16 data_size = strlen((char *)data);
     
     icmp *packet = create_icmp_packet(echo,data, data_size);
     // print_icmp_packet(packet, sizeof(icmp) + data_size);
     u8 *raw=create_raw_icmp(packet);
     // u16 size=sizeof(raw_icmp)+data_size;
     if(!raw){
          printf("Empty\n");
          exit(1);
     }
   
    
     struct addrinfo hints,*res;
     
     memset(&hints,0,sizeof(hints));

     hints.ai_family=AF_INET;
     hints.ai_socktype=SOCK_RAW;
     hints.ai_protocol=IPPROTO_ICMP;
     i32 status=getaddrinfo(argv[1],NULL,&hints,&res);
     
     if(status!=0){
          fprintf(stderr,RED"Error getting address info %s \n"RESET,gai_strerror(status));
          exit(EXIT_FAILURE);
     }

     char ip[INET_ADDRSTRLEN];


     struct sockaddr_in *addr=(struct sockaddr_in *)res->ai_addr;
     inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));
     IP *pkt=create_ip_packet(ICMP,3000,ip);

     pkt->payload=packet;

     u8 *raw_bytes=create_raw_ip(pkt);
 
    
     printf(GREEN"\nSending %hd bytes to %s \n"RESET,data_size,print_ip(inet_addr(ip)));

     STATS *stats=send_packets(pkt,&keep_sending);
     
     printf("\n");
     printf(YELLOW"---%s PINGv1 statistics---\n %lld packets transmitted, %lld received, %.1f%% packet loss,time %.1fms\n",
           print_ip(inet_addr(ip)),
           stats->packets_sent,
           stats->packets_received,
           100.0*(stats->packets_sent-stats->packets_received)/stats->packets_sent,
           stats->duration_ms
     );

     printf("rtt min/avg/max/mdev=%.3f/%.3f/%.3f/%.3f ms\n"RESET,stats->min_rtt,stats->avg_rtt,stats->max_rtt,stats->mdev_rtt);
     printf("\n");

     printf(WHITE"\t\tConnection Quality Interpretation\n");
     printf("\t--------------------------------------------------\n"RESET);

     if(stats->avg_rtt < 50 && stats->mdev_rtt < 10 &&
               (100.0 * (stats->packets_sent - stats->packets_received) / stats->packets_sent) == 0) {
          printf(GREEN"\tStatus      : EXCELLENT\n"RESET);
          printf(GREEN"\tDescription : Super responsive. Ideal for gaming, VoIP, and other latency-sensitive tasks.\n"RESET);
     } else if ((stats->avg_rtt >= 50 && stats->avg_rtt <= 100) &&
               stats->mdev_rtt < 20 &&
               (100.0 * (stats->packets_sent - stats->packets_received) / stats->packets_sent) < 1) {
          printf(CYAN"\tStatus      : GOOD\n"RESET);
          printf(CYAN"\tDescription : Reliable connection. Suitable for most online activities.\n"RESET);
     } else if ((stats->avg_rtt >= 100 && stats->avg_rtt <= 250) &&
               stats->mdev_rtt < 50 &&
               (100.0 * (stats->packets_sent - stats->packets_received) / stats->packets_sent) < 2) {
          printf(YELLOW"\tStatus      : MODERATE\n"RESET);
          printf(YELLOW"\tDescription : Acceptable for browsing and streaming. May struggle with real-time applications.\n"RESET);
     } else {
          printf(RED"\tStatus      : POOR\n"RESET);
          printf(RED"\tDescription : High latency or packet loss. Not suitable for demanding network tasks.\n"RESET);
     }

          printf(WHITE"\t--------------------------------------------------\n\n"RESET);


     //memory freeing
     free(stats);
     freeaddrinfo(res);
     free(pkt);
     free(raw_bytes);
     free(packet);
     free(raw);

}



void help(){
     
     printf(WHITE"\n\tUsage: ./main [options] <destination name or ip> \n");
     printf("\tOptions:\n");
     printf(" \t-c, --count <number of packets>  send a specific number of packets\n\n"RESET);
     exit(EXIT_SUCCESS);
  }

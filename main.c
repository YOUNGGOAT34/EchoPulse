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

     
   


     return 0;

}

void command_parser(i8 argc,i8 *argv[]){
    
     if(argc<2){
          help();
          exit(EXIT_SUCCESS);
     
     }

      
   static struct option long_options[]={
     {"count",required_argument,0,'c'},
     {"size",required_argument,0,'s'},
     {"quiet",no_argument,0,'q'},
     {"ttl",required_argument,0,'t'},
     {"timeout",required_argument,0,'W'},
     {"time",required_argument,0,'w'},
     {"interval",required_argument,0,'i'},
     {"help",no_argument,0,'h'},
     {0,0,0,0}
   };


   double_hyphen(argc,argv);

   i32 option;
   i32 options_index=0;

   options *opts=malloc(sizeof(options));
   opts->count=INT_MAX;
   opts->quiet=false;
   opts->payload_size=56;
   opts->ttl=255;
   opts->timeout=1000;
   opts->time=0;
   opts->interval=0;

   while((option=getopt_long(argc,argv,"c:hqs:t:W:w:i:",long_options,&options_index))!=-1){
        switch(option){
            case 'h':
               help();
               break;
            case 'c':
               opts->count=strtol(optarg,NULL,0);
               break;
            case 'q':
               opts->quiet=true;
               break;
            case 's':
               opts->payload_size=(u16)strtol(optarg,NULL,0);
               break;
            case 't':
               opts->ttl=parse_ttl((const i8 *) optarg);
               break;
            case 'W':
               opts->timeout=strtol(optarg,NULL,0);
               break;
            case 'w':
         
               opts->time=strtol(optarg,NULL,0);
               break;
            case 'i':
               opts->interval=strtol(optarg,NULL,0);
               break;
            default:
              fprintf(stderr,RED"Unknown option\n"RESET);

              exit(EXIT_FAILURE);
        }
   }


   if(optind>=argc){
          fprintf(stderr,RED"\n\tExpected a destination address ,Usage: ./main [options] <destination name or ip> \n\n"RESET);
          exit(EXIT_FAILURE);
     }

   /*
       Will handle the ctrl+c signal ,used to exit the program when the number of packets are not specified
   */
   signal(SIGINT,handle_sigInt);
    
   u8 *data =malloc(opts->payload_size);
   memset(data,'G',opts->payload_size);
   icmp *packet = create_icmp_packet(echo,data, opts->payload_size);
   u8 *raw=create_raw_icmp(packet);
 
   if(!raw){
        printf("Empty\n");
        exit(1);
   }
 
  
   struct addrinfo hints,*res;
   memset(&hints,0,sizeof(hints));

   hints.ai_family=AF_INET;
   hints.ai_socktype=SOCK_RAW;
   hints.ai_protocol=IPPROTO_ICMP;
   i32 status=getaddrinfo(argv[argc-1],NULL,&hints,&res);
   
   char ip[INET_ADDRSTRLEN];
   struct sockaddr_in *addr=(struct sockaddr_in *)res->ai_addr;
   inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));

   if(status!=0){
        fprintf(stderr,RED"Error getting address info %s \n"RESET,gai_strerror(status));
        exit(EXIT_FAILURE);
   }

   IP *pkt=create_ip_packet(ICMP,3000,ip);

   pkt->payload=packet;
//    u8 *raw_bytes=create_raw_ip(pkt,opts);

   STATS *stats;

   printf(GREEN"\nSending %hd bytes to %s \n"RESET,opts->payload_size,print_ip(inet_addr(ip)));

   if(opts->count==INT_MAX){
      stats=send_packets(pkt,&keep_sending,opts);
   }else if(opts->count>=0){
      stats=send_n_packets(pkt,opts,&keep_sending);
   }

   
   /*
      Formatted output 
   */

     printf("\n");
     printf(YELLOW"---%s EchoPulse statistics---\n %lld packets transmitted, %lld received, %.1f%% packet loss,time %.1fms\n",
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
     free(data);
     free(stats);
     freeaddrinfo(res);
     free(pkt);
     // free(raw_bytes);
     free(packet);
     free(raw);


}


void double_hyphen(i32 argc,i8 *argv[]){
   
     const char *long_opts[]={"count"};
     for(int i=0;i<argc;i++){
        
         if(argv[i][0]=='-' && argv[i][1]!='-' && strlen(argv[i])>2){
             int size=sizeof(long_opts)/sizeof(long_opts[0]);
             for(int j=0;j<size;j++){
                  if(strcmp(argv[i]+1,long_opts[j])==0){
                      fprintf(stderr,RED"Invalid option %s .Did you mean --%s?\n"RESET,argv[i],long_opts[j]);
                      
                      exit(EXIT_FAILURE);
                  }
             }
         }
     }
}



void help(){
     
     printf(WHITE"\n\tUsage: ./main [options] <destination name or ip> \n\n");
     printf("\tOptions:\n\n");
     printf(" \t-c, --count <number of packets>  send a specific number of packets\n\n");
     printf(" \t-q, --quiet   Quiet output ,only summary(statistics)\n\n");
     printf(" \t-s, --size <size>  specify the packet size\n\n");
     printf(" \t-t, --ttl <time>  specify the packet size\n\n");
     printf(" \t-i, --interval <time>  interval between packets in seconds\n\n");
     printf(" \t-w, --time <time>  session will automatically terminate after this duration,in miliseconds\n\n");
     printf(" \t-W, --timeout <time>  wait time per packet size\n\n"RESET);
     //add help for the total time the session will take

     exit(EXIT_SUCCESS);
   
  }


u8 parse_ttl(const i8 *ttl){
    errno=0;

    i64 val=strtol(ttl,NULL,0);

    if(errno!=0 || val<1 || val>255){
       fprintf(stderr,RED"Invalid ttl ,%s must be between 1 and 255\n"RESET,ttl);
       exit(EXIT_FAILURE);
    }

    return (u8)val;
}

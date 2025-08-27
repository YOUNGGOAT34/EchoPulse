#include "hexadump.h"


int main(){
     u8 *data = (u8 *)"Hello";
     u16 data_size = strlen((char *)data);
     
     icmp *packet = create_icmp_packet(echo,data, data_size);
     // print_icmp_packet(packet, sizeof(icmp) + data_size);

     u8 *raw=create_raw_icmp(packet);
     u16 size=sizeof(raw_icmp)+data_size;
     if(!raw){
          printf("Empty\n");
     }
     hexadump((void *)raw,size);
     


     return 0;
}
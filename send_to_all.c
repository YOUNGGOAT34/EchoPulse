#include "hexadump.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include "ip.h"
#include "send_to_all.h"



i32 get_iface_ip_mask(in_addr_t *mask,in_addr_t *current_ip) {
   // char *iface="wlan0";
   i32 fd = socket(AF_INET, SOCK_DGRAM, 0);
   if (fd < 0) return -1;
   
   struct ifreq if_request;
   strncpy(if_request.ifr_name,"wlan0", IFNAMSIZ);

   // Get IP
   if (ioctl(fd, SIOCGIFADDR, &if_request) < 0) return -1;
   *current_ip = ((struct sockaddr_in *)&if_request.ifr_addr)->sin_addr.s_addr;

   // Get Netmask
   if (ioctl(fd, SIOCGIFNETMASK, &if_request) < 0) return -1;
   *mask = ((struct sockaddr_in *)&if_request.ifr_netmask)->sin_addr.s_addr;

   close(fd);
   return 0;
}


void compute_subnet_range(in_addr_t ip, in_addr_t mask) {
   in_addr_t start_ip_address = ip & mask;
   in_addr_t end_ip_address = start_ip_address | ~mask;
   start_ip_address = htonl(ntohl(start_ip_address) + 1);
   end_ip_address = htonl(ntohl(end_ip_address) - 1);

   printf("Start ip: %s\n",print_ip(start_ip_address));
   printf("End Ip address: %s\n",print_ip(end_ip_address));
}

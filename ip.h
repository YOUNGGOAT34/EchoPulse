#ifndef IP_H
#define IP_H

#include <arpa/inet.h>
#include "icmp.h"

typedef struct IP{
     u8 type:3;
     u16 id;
     u32 src;
     u32 dst;
     icmp *payload;
      
} __attribute__((packed)) IP;

typedef struct raw_IP{
     u8 ihl:4;
     u8 version:4;
     u8 dscp:6;
     u8 ecn:2;
     u16 length;
     u16 id;
     u8 flags:3;
     u16 fragmet_offset:13;
     u8 TTL;
     u8 protocol;
     u16 checksum;
     u32 src;
     u32 dst;
     u8 options[];

}__attribute__((packed)) RAWIP;


IP *create_ip_packet(const u8 type,u16 id,const i8 * dst);
u32 get_local_ip(void);
void print_ip_packet(IP *);
u8 *create_raw_ip(IP *);
void print_ip(u32 );
#endif
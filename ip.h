#ifndef IP_H
#define IP_H

#include <arpa/inet.h>
#include "ping.h"

typedef struct IP{
     u8 type:4;
     u16 id;
     u32 src;
     u32 dst;
      
}IP __attribute__((packed));


IP *create_ip(const u8 type,u16 id,const i8 * dst);
u32 get_local_ip(void);
#endif
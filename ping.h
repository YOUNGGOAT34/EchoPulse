#ifndef PING_H
#define PING_H

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/socket.h>
#include "string.h"
#include "errno.h"
#include <unistd.h>
#include <netinet/in.h>



#define RED   "\033[1;31m"
#define GREEN   "\033[1;32m"
#define RESET "\033[0m"

typedef unsigned long long int u64;
typedef unsigned int u32;
typedef unsigned short int u16;
typedef unsigned char u8;
typedef char i8;

typedef struct h_icmp{
     u8 type;
     u8 code;
     u16 checksum;
     u8 data[];

}__attribute__((packed)) icmp;

icmp *create_icmp_packet(u8 type,u8 code,u8 *data,u16 size);
void error(const i8* msg);
u16 checksum(icmp *,u16 size);
u16 _checksum(u8 *data,size_t n);
void print_icmp_packet(icmp *packet,u16);

#endif


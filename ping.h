#ifndef PING_H
#define PING_H

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/socket.h>
#include "string.h"
#include "errno.h"
#include <netinet/in.h>

//colors to use

#define RED   "\033[1;31m"
#define GREEN   "\033[1;32m"
#define RESET "\033[0m"

typedef unsigned long long int uint64;
typedef unsigned int uint32;
typedef unsigned short int uint16;
typedef unsigned char uint8;
typedef char int8;

typedef struct h_icmp{
     uint8 type;
     uint8 code;
     uint16 checksum;
     uint8 data[];

}__attribute__((packed)) icmp;

icmp *create_icmp_packet(uint8 type,uint8 code,uint8 *data,uint16 size);
void error(const int8* msg);
uint16 checksum(icmp *,uint16 size);
uint16 _checksum(uint8 *data,size_t n);
void print_icmp_packet(icmp *packet,uint16);

#endif


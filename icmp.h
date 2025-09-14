#ifndef ICMP_H
#define ICMP_H

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/socket.h>
#include "string.h"
#include "errno.h"
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <limits.h>
#include <netdb.h>




#define RED   "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW "\e[0;33m"
#define RESET "\033[0m"
#define CYAN "\033[1;36m"


typedef unsigned long long int u64;
typedef unsigned int u32;
typedef unsigned short int u16;
typedef unsigned char u8;

typedef char i8;
typedef signed long long int i64;
typedef signed int i32;
typedef signed short int i16;


typedef enum{
    echo,
    reply,
    ICMP
}TYPE;


typedef struct h_icmp{
     TYPE type;
     u8 *data;
     u16 size;

}__attribute__((packed)) icmp;


typedef struct raw_icmp{
     u8 type;
     u8 code;
     u16 checksum;
     u16 id;
     u16 sequence;
     u8 data[];
}__attribute__((packed)) raw_icmp;

icmp *create_icmp_packet(TYPE type,u8 *data,u16 size);
u8 *create_raw_icmp(icmp *packet);
void error(const i8* msg);
u16 checksum(u8 *data,size_t n);
void print_icmp_packet(icmp *packet,u16);


#endif


#ifndef HEXADUMP_H
#define HEXADUMP_H
#include "icmp.h"


#define IS_PRINTABLE_ASCII(c) (((c)>31) && ((c)<127))

void hexadump(void *buff,u16 size);
#endif
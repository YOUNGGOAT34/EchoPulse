#ifndef MAIN_H
#define MAIN_H

#include "hexadump.h"
#include "send_raw.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <getopt.h>




int main(i32,i8 *[]);
void command_parser(i8 ,i8 *[]);
void handle_sigInt(__attribute__((unused)) i32 sig);
void help(void);
void double_hyphen(i32,i8 *[]);
u8 parse_ttl(const i8 *);

#endif
#ifndef MAIN_H
#define MAIN_H




int main(i32,i8 *[]);
void command_parser(i32,i8 *[]);
void handle_sigInt(__attribute__((unused)) i32 sig);
void help(void);
void double_hyphen(i32,i8 *[]);
u8 parse_ttl(const i8 *);

#endif
#ifndef MAIN_H
#define MAIN_H

typedef struct cmd_args{
  char** args;
  int len;
}cmd_args;
void command(char* cmd);

//tools
cmd_args* __get_args(char* cmd);
#endif

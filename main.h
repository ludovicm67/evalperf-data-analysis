#ifndef __MAIN_H
#define __MAIN_H

#ifndef DEBUG
#define DEBUG 0
#endif

#define BUFF_SIZE 1024

typedef enum { false = 0, true } bool;

struct params {
  char *mat_file;
  char *trace_file;
  int trace_fd;
  int nb_nodes;
  int nb_codes[5]; // indexes representing code from 1 to 4
};

struct params new_params();

#endif

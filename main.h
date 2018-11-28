#ifndef __MAIN_H
#define __MAIN_H

#include <stdbool.h>
#include <stdio.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define BUFF_SIZE 1024

struct evt {
  double t;
  int code;
  int pos;

  struct evt *next;
};

struct paquet {
  int fid;
  int pid;
  int source;
  int dest;
  double size;
  double start;
  double end;
  double t;

  struct evt *evts;
  struct paquet *next;
};

struct params {
  char *mat_file;
  char *trace_file;
  int trace_fd;
  int nb_nodes;
  int nb_codes[5]; // indexes representing code from 1 to 4
  struct list *flow_list;
  int *mat;
  FILE *mat_stream;

  int trace_paquet; // will contain the packet id to trace, or -1 if not set
  struct paquet
      *traced_paquet; // only set if we want to trace a specific paquet

  int trace_flow; // will contain the flow id to trace, or -1 if not set
};

struct params new_params();

#endif

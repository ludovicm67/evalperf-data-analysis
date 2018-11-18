#include "main.h"
#include "matrix.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct flow *flow_list = NULL;

struct flow {
  int fid;
  bool destroyed;
  double begin;
  double end;
  struct flow *next;
};

struct flow *new_flow(int fid, double begin) {
  struct flow *f = malloc(sizeof(struct flow));
  f->fid = fid;
  f->destroyed = false;
  f->begin = begin;
  f->end = begin;
  f->next = NULL;
  return f;
}

void free_flow(struct flow *f) {
  if (f == NULL)
    return;
  free_flow(f->next);
  free(f);
}

void add_flow(int fid, double ts) {
  struct flow *f;

  // if it is the first flow, initialize it
  if (flow_list == NULL) {
    flow_list = new_flow(fid, ts);
    return;
  }

  // loop through the list to find if it already exists or not
  for (f = flow_list; f->next != NULL; f = f->next) {
    if (f->fid == fid) {
      f->end = ts;
      return;
    }
  }

  // if it's a new flow
  f = new_flow(fid, ts);
  f->next = flow_list;
  flow_list = f;
}

int get_nb_flows(struct flow *f) {
  int res;
  if (f == NULL)
    return 0;
  res = 0;
  while (f->next) {
    res++;
    f = f->next;
  }
  return res;
}

void treat_line(char *line, struct params *p) {
  double t;
  int code, pid, fid, tos, bif, s, d, pos;

  // if we don't have 9 columns
  if (sscanf(line, "%lf %d %d %d %d %d N%d N%d N%d", &t, &code, &pid, &fid,
             &tos, &bif, &s, &d, &pos) != 9) {
    // ...and if the columns number is also different from 8
    if ((sscanf(line, "%lf %d %d %d %d N%d N%d N%d", &t, &code, &pid, &fid,
                &tos, &s, &d, &pos)) != 8) {
      // ...tell the user that it is an error!
      fprintf(stderr,
              "sscanf: error when trying to read the following content: '%s'\n",
              line);
      exit(EXIT_FAILURE);
    }
  }

#if DEBUG
  printf(
      "t=%f, code=%d, pid=%d, fid=%d, tos=%d, bif=%d, s=N%d, d=N%d, pos=N%d\n",
      t, code, pid, fid, tos, bif, s, d, pos);
#endif

  if (code < 0 || code > 4) {
    fprintf(stderr, "wrong code: got %d instead something between 0 and 4\n",
            code);
    exit(EXIT_FAILURE);
  }
  p->nb_codes[code]++;

  switch (code) {
  case 0:
  case 3:
  case 4:
    add_flow(fid, t);
    break;
  }
}

void treat_file(struct params *p) {
  char buffer[BUFF_SIZE + 1];
  char *tmp;
  char *line;
  char *line_tmp;
  int nb_read = 0;
  int offset;

  while ((nb_read = read(p->trace_fd, buffer, BUFF_SIZE)) > 0) {
    tmp = buffer + nb_read;
    offset = 0;

    while (*(--tmp) != '\n')
      offset--;
    *(++tmp) = '\0';

    if (lseek(p->trace_fd, offset, SEEK_CUR) < 0) {
      fprintf(stderr, "lseek: failed");
      exit(EXIT_FAILURE);
    }

    line = buffer;
    line_tmp = line;
    while (*line_tmp) {
      if (*line_tmp == '\n') {
        *line_tmp = '\0';
        treat_line(line, p);
        line = ++line_tmp;
      } else {
        line_tmp++;
      }
    }

    if (nb_read < BUFF_SIZE)
      break;
  }
}

// init params with default values
struct params new_params() {
  struct params a;
  a.mat_file = "res26.txt";
  a.trace_file = "trace2650.txt";
  a.nb_codes[0] = 0;
  a.nb_codes[1] = 0;
  a.nb_codes[2] = 0;
  a.nb_codes[3] = 0;
  a.nb_codes[4] = 0;
  a.nb_nodes = 1;
  return a;
}

int main(int argc, char *argv[]) {
  struct params a = new_params();
  int c;
  int nb_errors;

  // get arguments
  while ((c = getopt(argc, argv, "+f:m:")) != EOF) {
    switch (c) {
    case 'f':
      a.trace_file = optarg;
      break;
    case 'm':
      a.mat_file = optarg;
      break;
    case '?':
      nb_errors++;
      break;
    }
  }

  // char *tmp;
  // char *line = NULL;
  // int i, j; // counters
  // size_t len = 0;
  FILE *mat_stream = fopen(a.mat_file, "r");
  if (mat_stream == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  a.nb_nodes = get_nb_nodes(mat_stream);

  // int mat[a.nb_nodes][a.nb_nodes];
  // for (i = 0; i < a.nb_nodes; i++) {
  //   if (getline(&line, &len, mat_stream) == -1) {
  //     fprintf(stderr, "matrix file is empty\n");
  //     exit(EXIT_FAILURE);
  //   }
  //   tmp = line;
  //   for (j = 0; j < a.nb_nodes; j++) {
  //     mat[i][j] = atoi(strtok(tmp, " \t"));
  //     tmp = NULL;
  //   }
  // }
  // free(line);
  fclose(mat_stream);

  if ((a.trace_fd = open(a.trace_file, O_RDONLY)) < 0) {
    fprintf(stderr, "open: failure\n");
    exit(EXIT_FAILURE);
  }

  treat_file(&a);

  if (close(a.trace_fd) < 0) {
    fprintf(stderr, "close: failure\n");
    exit(EXIT_FAILURE);
  }

  // global data
  int nb_events = a.nb_codes[0] + a.nb_codes[1] + a.nb_codes[2] +
                  a.nb_codes[3] + a.nb_codes[4];
  printf("Number of nodes: %d\n", a.nb_nodes);
  printf("Number of flows: %d\n", get_nb_flows(flow_list));
  printf("Number of events: %d\n", nb_events);
  printf("Number of packets emited (code 0): %d\n", a.nb_codes[0]);
  printf("Number of packets processed (code 2): %d\n", a.nb_codes[2]);
  printf("Number of packets received (code 3): %d\n", a.nb_codes[3]);
  printf("Number of destroyed packets (code 4): %d\n", a.nb_codes[4]);
  printf("Number of packets lost (code 0 - 3): %d\n",
         a.nb_codes[0] - a.nb_codes[3]);
  printf("Lost rate: %f%%\n",
         ((float)(a.nb_codes[0] - a.nb_codes[3]) / a.nb_codes[0]) * 100);

  free_flow(flow_list);

  exit(EXIT_SUCCESS);
}

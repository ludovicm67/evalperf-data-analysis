#include "main.h"
#include "flow.h"
#include "matrix.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    add_flow(p, fid, t);
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
  struct params p;
  p.mat_file = "res26.txt";
  p.trace_file = "trace2650.txt";
  p.nb_codes[0] = 0;
  p.nb_codes[1] = 0;
  p.nb_codes[2] = 0;
  p.nb_codes[3] = 0;
  p.nb_codes[4] = 0;
  p.nb_nodes = 1;
  p.flow_list = NULL;
  return p;
}

int main(int argc, char *argv[]) {
  struct params p = new_params();
  int c;
  int nb_errors;

  // get arguments
  while ((c = getopt(argc, argv, "+f:m:")) != EOF) {
    switch (c) {
    case 'f':
      p.trace_file = optarg;
      break;
    case 'm':
      p.mat_file = optarg;
      break;
    case '?':
      nb_errors++;
      break;
    }
  }

  init_matrix(&p);

  if ((p.trace_fd = open(p.trace_file, O_RDONLY)) < 0) {
    fprintf(stderr, "open: failure\n");
    exit(EXIT_FAILURE);
  }

  treat_file(&p);

  if (close(p.trace_fd) < 0) {
    fprintf(stderr, "close: failure\n");
    exit(EXIT_FAILURE);
  }

  // global data
  int nb_events = p.nb_codes[0] + p.nb_codes[1] + p.nb_codes[2] +
                  p.nb_codes[3] + p.nb_codes[4];
  printf("Number of nodes: %d\n", p.nb_nodes);
  printf("Number of flows: %d\n", get_nb_flows(p.flow_list));
  printf("Number of events: %d\n", nb_events);
  printf("Number of packets emited (code 0): %d\n", p.nb_codes[0]);
  printf("Number of packets processed (code 2): %d\n", p.nb_codes[2]);
  printf("Number of packets received (code 3): %d\n", p.nb_codes[3]);
  printf("Number of destroyed packets (code 4): %d\n", p.nb_codes[4]);
  printf("Number of packets lost (code 0 - 3): %d\n",
         p.nb_codes[0] - p.nb_codes[3]);
  printf("Lost rate: %f%%\n",
         ((float)(p.nb_codes[0] - p.nb_codes[3]) / p.nb_codes[0]) * 100);

  free_flow(p.flow_list);

  print_matrix(&p);
  free_matrix(&p);

  exit(EXIT_SUCCESS);
}

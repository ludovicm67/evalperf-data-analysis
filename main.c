#include "main.h"
#include "flow.h"
#include "list.h"
#include "matrix.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct evt *evt_new(double t, int code, int pos) {
  struct evt *e = malloc(sizeof(struct evt));
  e->t = t;
  e->code = code;
  e->pos = pos;
  e->next = NULL;
  return e;
}

struct paquet *paquet_new(int fid, int pid, int source, int dest,
                          double start) {
  struct paquet *p = malloc(sizeof(struct paquet));
  p->fid = fid;
  p->pid = pid;
  p->source = source;
  p->dest = dest;
  p->size = 0;
  p->start = start;
  p->end = start;
  p->evts = NULL;
  p->next = NULL;
  return p;
}

void evt_free(struct evt *e) {
  if (!e)
    return;
  evt_free(e->next);
  free(e);
}

void paquet_free(struct paquet *p) {
  if (!p)
    return;
  evt_free(p->evts);
  paquet_free(p->next);
  free(p);
}

void evt_print(struct evt *e) {
  if (!e)
    return;
  struct evt *t = e;
  double ts = t->t;
  double diff_time;
  for (t = e; t; t = t->next) {
    diff_time = t->t - ts;
    printf("    - [t=%f] ", t->t);
    switch (t->code) {
    case 0:
      printf("CREATION (code=%d) on node N%d\n", t->code, t->pos);
      break;
    case 1:
      printf("NODE_ARRIVAL (code=%d) on node N%d. ", t->code, t->pos);
      if (diff_time) {
        printf("Duration %f sec.\n", diff_time);
      } else {
        printf("Instant.\n");
      }
      break;
    case 2:
      printf("LEAVE_QUEUE (code=%d), going to N%d. ", t->code, t->pos);
      if (diff_time) {
        printf("Waited %f sec in the queue.\n", diff_time);
      } else {
        printf("No waittime.\n");
      }
      break;
    case 3:
      printf("REACHED_DESTINATION (code=%d) which is N%d\n", t->code, t->pos);
      break;
    case 4:
      printf("DESTROYED (code=%d) on node N%d\n", t->code, t->pos);
      break;
    default:
      printf("UNKNOWN_EVENT (code=%d) on node N%d\n", t->code, t->pos);
    }
    ts = t->t;
  }
}

void paquet_add_evt(struct params *params, struct paquet *p, double t, int code,
                    int pos) {
  struct evt *e;
  int i;
  if (!p)
    return;
  p->end = t;
  if (!p->evts) {
    p->evts = evt_new(t, code, pos);
    return;
  }
  i = 0;
  for (e = p->evts; e->next; e = e->next)
    i++;
  e->next = evt_new(t, code, pos);

  if (!p->size && i == 1) { // compute paquet size directly when we can
    p->size = (p->end - p->start) * matrix_get(params, p->source, pos);
  }
}

void paquet_print(struct paquet *p) {
  if (!p) {
    printf(" /!\\ Paquet not found.\n");
    return;
  }
  if (p->next)
    paquet_print(p->next);
  printf("\n\n========= PID = %d =========\n", p->pid);
  printf(" - fid: %d\n", p->fid);
  printf(" - source: N%d\n", p->source);
  printf(" - destination: N%d\n", p->dest);
  printf(" - life start: t=%f\n", p->start);
  printf(" - end of life: t=%f\n", p->end);
  printf(" - lifetime: %f sec\n", p->end - p->start);
  printf(" - size: %f Mb\n", p->size);
  if (p->evts) {
    printf(" - events:\n");
    evt_print(p->evts);
  }
}

void treat_line(char *line, struct params *p) {
  char buf[BUFF_SIZE];
  double t;
  int code, pid, fid, tos, bif, s, d, pos;
  struct flow *f;

  // if we don't have 9 columns
  if (sscanf(line, "%lf %d %d %d %d %d N%d N%d N%d", &t, &code, &pid, &fid,
             &tos, &bif, &s, &d, &pos) != 9) {
    // ...and if the columns number is also different from 8
    if ((sscanf(line, "%lf %d %d %d %d N%d N%d N%d", &t, &code, &pid, &fid,
                &tos, &s, &d, &pos)) != 8) {
      // ...tell the user that it is an error
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

  // if we need to trace this paquet
  if (p->trace_paquet == pid) {
    if (!p->traced_paquet)
      p->traced_paquet = paquet_new(fid, pid, s, d, t);
    paquet_add_evt(p, p->traced_paquet, t, code, pos);
  }

  if (code == 4) {
    sprintf(buf, "%f %d\n", t, p->nb_codes[4]);
    fwrite(buf, 1, strlen(buf), p->graph_paquet_lost);
  }

  if (code == 0 || code == 3 || code == 4) {
    // graph: nb paquets
    sprintf(buf, "%f %d\n", t,
            p->nb_codes[0] - p->nb_codes[3] - p->nb_codes[4]);
    fwrite(buf, 1, strlen(buf), p->graph_paquets);
    // create/update flow
    flow_add(p, fid, t);
  }

  f = p->flow_list->l[fid];
  f->nb_codes[code]++;
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
  p.flow_list = list_new();
  p.trace_paquet = -1;
  p.trace_flow = -1;

  if ((p.graph_paquets = fopen("./graph_nb_paquets.txt", "w+")) == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  if ((p.graph_paquet_lost = fopen("./graph_lost_paquets.txt", "w+")) == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  return p;
}

int main(int argc, char *argv[]) {
  struct params p = new_params();
  int c;
  int nb_errors = 0;

  // get arguments
  while ((c = getopt(argc, argv, "+t:m:p:f:")) != EOF) {
    switch (c) {
    case 't':
      p.trace_file = optarg;
      break;
    case 'm':
      p.mat_file = optarg;
      break;
    case 'p':
      p.trace_paquet = atoi(optarg);
      break;
    case 'f':
      p.trace_flow = atoi(optarg);
      break;
    case '?':
      nb_errors++;
      break;
    }
  }

  if (nb_errors) {
    fprintf(stderr,
            "usage: %s -t trace_file -m matrix_file -p packet_id -f flow_id",
            argv[0]);
    exit(EXIT_FAILURE);
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
  printf("Number of flows: %d\n", flow_nb(&p));
  printf("Number of events: %d\n", nb_events);
  printf("Number of packets emited (code 0): %d\n", p.nb_codes[0]);
  printf("Number of packets arrivals (code 1): %d\n", p.nb_codes[1]);
  printf("Number of packets processed (code 2): %d\n", p.nb_codes[2]);
  printf("Number of packets received (code 3): %d\n", p.nb_codes[3]);
  printf("Number of destroyed packets (code 4): %d\n", p.nb_codes[4]);
  printf("Number of packets lost (code 0 - 3): %d\n",
         p.nb_codes[0] - p.nb_codes[3]);
  printf("Loss rate: %f%%\n",
         ((float)(p.nb_codes[0] - p.nb_codes[3]) / p.nb_codes[0]) * 100);

  // if the user asked to trace a specific paquet, display results
  if (p.trace_paquet != -1) {
    printf("\n\n\nDisplaying traced paquet (#%d) as requested:",
           p.trace_paquet);
    paquet_print(p.traced_paquet);
  }

  // if the user asked to trace a specific flow, display results
  if (p.trace_flow != -1) {
    printf("\n\n\nDisplaying traced flow (#%d) as requested:", p.trace_flow);
    flow_print(&p, p.trace_flow);
  }

  list_free(p.flow_list, free);

#if DEBUG
  print_matrix(&p);
#endif

  free_matrix(&p);
  paquet_free(p.traced_paquet);

  // close files used to generate data for gnuplot
  fclose(p.graph_paquets);
  fclose(p.graph_paquet_lost);

  exit(EXIT_SUCCESS);
}

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define BUFF_SIZE 2048

int nb_codes[] = {0, 0, 0, 0, 0}; // indexes from 0 to 4

int nb_events = 0;
int file_size = 0;
int searching_file_size = 1; // or 0

double t;
int code;
int pid;
int fid;
int tos;
int bif;
int s;
int d;
int pos;

void treat_line(char *line) {
  // si on a pas 9 colonnes
  if (sscanf(line, "%lf %d %d %d %d %d N%d N%d N%d", &t, &code, &pid, &fid,
             &tos, &bif, &s, &d, &pos) != 9) {
    // ...et si le nombre de colonnes n'est pas égal à 8
    if ((sscanf(line, "%lf %d %d %d %d N%d N%d N%d", &t, &code, &pid, &fid,
                &tos, &s, &d, &pos)) != 8) {
      // ...alors c'est une erreur
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

  nb_events++;

  if (code < 0 || code > 4) {
    fprintf(stderr, "wrong code: got %d instead something between 0 and 4\n",
            code);
    exit(EXIT_FAILURE);
  }
  nb_codes[code]++;

  if (searching_file_size && pos == 4) {
    switch (code) {
    case 1:
      // case 3:
      file_size--;
      break;
    // case 0:
    case 2:
      file_size++;
      break;
    case 4:
      searching_file_size = 0;
      break;
    default:
      break;
    }
  }

  return;
}

void treat_file(int fd) {
  char buffer[BUFF_SIZE + 1];
  char *tmp;
  char *line;
  char *line_tmp;
  int nb_read = 0;
  int offset;

  while ((nb_read = read(fd, buffer, BUFF_SIZE)) > 0) {
    tmp = buffer + nb_read;
    offset = 0;

    while (*(--tmp) != '\n')
      offset--;
    *(++tmp) = '\0';

    if (lseek(fd, offset, SEEK_CUR) < 0) {
      fprintf(stderr, "lseek: failed");
      exit(EXIT_FAILURE);
    }

    line = buffer;
    line_tmp = line;
    while (*line_tmp) {
      if (*line_tmp == '\n') {
        *line_tmp = '\0';
        treat_line(line);
        line = ++line_tmp;
      } else {
        line_tmp++;
      }
    }

    if (nb_read < BUFF_SIZE)
      break;
  }
}

int main() {

  char *trace_file = "trace2650.txt";
  int trace_fd;

  if ((trace_fd = open(trace_file, O_RDONLY)) < 0) {
    fprintf(stderr, "open: failure\n");
    exit(EXIT_FAILURE);
  }

  treat_file(trace_fd);

  if (close(trace_fd) < 0) {
    fprintf(stderr, "close: failiure\n");
    exit(EXIT_FAILURE);
  }

  printf("Nombre total d'évènements : %d\n", nb_events);
  printf("Nombre total de paquets émis : %d\n", nb_codes[0]);
  printf("Nombre total de paquets perdus : %d\n", nb_codes[0] - nb_codes[3]);
  printf("Taux de perte : %f%%\n",
         ((float)(nb_codes[0] - nb_codes[3]) / nb_codes[0]) * 100);
  printf("Nombre total de paquets détruits (code 4) : %d\n", nb_codes[4]);
  // printf("Taille de la file de N4 : %d\n", file_size);

  exit(EXIT_SUCCESS);
}

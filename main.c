#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int parse_file(char *, long int, long int, long int);
void split_chunks(char *, int, int);
void *threaded_parser(void *);
void read_full_file(char *);

int main(void) {
  printf("Mate fuk is up\n");
  split_chunks("data.txt", 256, 10000000);
  // read_full_file("data.txt");
  return 1;
}

void read_full_file(char *filename) {
  FILE *fp = fopen(filename, "r");
  fseek(fp, 0L, SEEK_END);
  long int size = ftell(fp);
  rewind(fp);
  char *memory = (char *)malloc(size);
  fread(memory, size, 1, fp);
  printf("Nkn eno idu\n");
  return;
}

struct TaskArgs {
  long int start;
  long int end;
  char *filename;
  long int maxLines;
  long int lines_counted;
};

void split_chunks(char *filename, int n, int nlines) {
  FILE *fd = fopen(filename, "r");
  fseek(fd, 0L, SEEK_END);
  long int size = ftell(fd);
  long int chunk_size = size / n;
  printf("Chunk size is %ld, size is %ld\n", chunk_size, size);
  fseek(fd, 0L, SEEK_END);

  long int prevChunk = 0;
  pthread_t tasks[n];
  struct TaskArgs task_args[n];
  fclose(fd);
  for (int i = 0; i < n; i++) {
    task_args[i] = (struct TaskArgs){prevChunk, prevChunk + chunk_size,
                                     filename, nlines / n, 0};
    pthread_create(&tasks[i], NULL, threaded_parser, &task_args[i]);
    // threaded_parser(&task_args[i]);
    prevChunk += chunk_size;
  }
  for (int i = 0; i < n; i++) {
    pthread_join(tasks[i], NULL);
  }
  int total = 0;
  for (int i = 0; i < n; i++) {
    total += task_args[i].lines_counted;
  }
  printf("Total shit down the hole is %d\n", total);
}

void *threaded_parser(void *args) {
  struct TaskArgs *task = args;
  clock_t begin = clock();
  int lines =
      parse_file(task->filename, task->start, task->end, task->maxLines);
  task->lines_counted = lines;
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Spent time in thread %f\n", time_spent);
  pthread_exit(NULL);
}

int parse_file(char *filename, long int start, long int end,
               long int maxLines) {

  FILE *fd = fopen(filename, "r");
  if (fd == NULL) {
    perror("Fo I died");
  }
  fseek(fd, start, SEEK_SET);

  char *line = NULL;
  size_t len, read = 0;
  long int count = 0;

  char city[256];
  double temp = 0.0;

  // while ((read = getline(&line, &len, fd)) != -1) {
  //   sscanf(line, "%[^;];%lf", city, &temp);
  //   // sscanf(line, "%[^;];%lf", city, &temp);
  //   count++;
  //   if (ftell(fd) >= end) {
  //     break;
  //   }
  // }
  while (fscanf(fd, "%[^;];%lf", city, &temp) == 2) {
    count++;
    if (ftell(fd) > end) {
      break;
    }
  }

  fclose(fd);
  if (line)
    free(line);
  return count;
}

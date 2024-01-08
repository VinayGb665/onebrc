#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct calculations {
  float sum_map[26];
  float max_map[26];
  float min_map[26];
  int count_map[26];
} calculations;

struct task {
  FILE *fp;
  long int start;
  long int end;
  long int counter;
  calculations calc;
};
int parse_file(FILE *, long int, long int, long int);
void split_chunks(char *, int, int);
void *threaded_parser(void *);
void read_full_file(char *);
struct task *prepare_tasks(char *, int);
void *executeChunkTask(void *);
void runner(struct task *, int);
void print_report(calculations *);

void mergeCalculations(calculations *, calculations *, calculations *);
void processChunk(struct task *);

int main(void) {
  printf("Mate fuk is up\n");
  // split_chunks("data.txt", 256, 10000000);
  int workers = 1;
  struct task *t = prepare_tasks("../data-smol.txt", workers);
  runner(t, workers);
  return 1;
}

void runner(struct task *tasks, int workers) {

  calculations result = {{0.0}, {0.0}, {0.0}, {0}};
  pthread_t worker_threads[workers];
  for (int i = 0; i < workers; i++) {
    pthread_create(&worker_threads[i], NULL, executeChunkTask, &tasks[i]);
  }
  for (int i = 0; i < workers; i++) {
    pthread_join(worker_threads[i], NULL);
  }

  long int total = 0;
  for (int j = 0; j < workers; j++) {
    total += tasks[j].counter;
    mergeCalculations(&tasks[j].calc, &result, &result);
  }
  print_report(&result);

  printf("SMD I read %ld  %d lines \n", total, result.count_map[0]);
}

struct task create_task(char *filename, long int start, long int end) {
  FILE *fp = fopen(filename, "rb");
  fseek(fp, start, 0);

  calculations x = {{0.0}, {0.0}, {0.0}, {0}};
  return (struct task){fp, start, end, 0, x};
}

void *executeChunkTask(void *arg) {
  struct task *t = arg;
  processChunk(t);
  pthread_exit(NULL);
}

float parse_temperature(char *temperature) {
  int neg = 0;
  if (temperature[0] == '-') {
    temperature++;
    neg = 1;
  }
  int delim_index = strcspn(temperature, ".");
  int decimalPart = temperature[delim_index + 1] - '0';
  temperature[delim_index] = '\0';
  int intPart = atoi(temperature);
  float temp = intPart + ((float)decimalPart / 10);
  if (neg == 1) {
    return temp * -1;
  }
  return temp;
}
void mergeCalculations(calculations *x, calculations *y, calculations *result) {

  for (int i = 0; i < 26; i++) {
    result->sum_map[i] = x->sum_map[i] + y->sum_map[i];
    result->count_map[i] = x->count_map[i] + y->count_map[i];
    result->max_map[i] =
        x->max_map[i] > y->max_map[i] ? x->max_map[i] : y->max_map[i];
    result->min_map[i] =
        x->min_map[i] < y->min_map[i] ? x->min_map[i] : y->min_map[i];
  }
}

void processCalculations(calculations *x, int city_index, float temperature) {
  x->sum_map[city_index] = x->sum_map[city_index] + temperature;
  x->count_map[city_index] = x->count_map[city_index] + 1;
  if (x->max_map[city_index] < temperature) {
    x->max_map[city_index] = temperature;
  }

  if (x->min_map[city_index] > temperature) {
    x->min_map[city_index] = temperature;
  }
}

void print_report(calculations *x) {
  for (int i = 0; i < 26; i++) {
    if (x->count_map[i] > 0) {
      printf("MIN for %c is %.1f\n", (char)i + 'A', x->min_map[i]);
      printf("MAX for %c is %.1f\n", (char)i + 'A', x->max_map[i]);
      printf("MEAN for %c is %.1f\n", (char)i + 'A',
             x->sum_map[i] / x->count_map[i]);

      printf("SUM for %c is %d\n", (char)i + 'A', x->count_map[i]);
      printf("----------------\n");
    }
  }
}

void processChunk(struct task *t) {
  struct {
    char *key;
    float value[2];
  } *sumMap = NULL;
  FILE *fp = t->fp;
  fseek(fp, t->start, SEEK_SET);
  char line[16];

  calculations *x = &t->calc;
  long int counter = 0;
  while (ftell(fp) < t->end) {
    if (fgets(line, 16, fp) != NULL) {
      int delim_index = strcspn(line, ";");
      counter++;
      char *tempS = &line[0] + delim_index + 1;
      if (tempS != NULL) {
        float temp = parse_temperature(tempS);
        int index = line[0] - 'A';
        processCalculations(x, index, temp);
      }
    }
  }
  t->counter = counter;
  fclose(t->fp);
  return;
}

struct task *prepare_tasks(char *filename, int workers) {

  FILE *fp = fopen(filename, "rb");
  fseek(fp, 0L, SEEK_END);
  long int size = ftell(fp);
  rewind(fp);
  long int chunkSize = size / workers;
  long int base = 0L;
  struct task *tasks = (struct task *)malloc(sizeof(struct task) * workers);
  int chunkNumber = 0;
  for (;;) {
    long int chunkEnd = base + chunkSize;
    if (chunkEnd >= size) {
      tasks[chunkNumber] = create_task(filename, base, size);
      break;
    }

    fseek(fp, chunkEnd, SEEK_SET);
    char buf;
    for (;;) {
      buf = getc(fp);
      if (buf == '\n') {
        // printf("Broke because of newline at %ld\n", chunkEnd);
        break;
      }

      chunkEnd++;
    }

    tasks[chunkNumber] = create_task(filename, base, chunkEnd);
    base = chunkEnd + 1;
    chunkNumber++;
  }
  printf("Maga So i created %d tasks with size %ld\n", chunkNumber, size);
  // for (int i = 0; i < workers; i++) {
  //   printf("Task(%d) starts at %ld, ends at %ld\n", i, tasks[i].start,
  //          tasks[i].end);
  // }
  return tasks;
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
  FILE *fd;
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
  rewind(fd);
  for (int i = 0; i < n; i++) {
    task_args[i] =
        (struct TaskArgs){prevChunk, prevChunk + chunk_size, fd, nlines / n, 0};
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
  int lines = parse_file(task->fd, task->start, task->end, task->maxLines);
  task->lines_counted = lines;
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Spent time in thread %f\n", time_spent);
  pthread_exit(NULL);
}

int parse_file(FILE *fp, long int start, long int end, long int maxLines) {

  FILE *fd = fdopen(dup(fileno(fp)), "r");
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

  if (line)
    free(line);
  return count;
}

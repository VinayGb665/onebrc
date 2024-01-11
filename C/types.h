
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct calculations {
  float sum_map[26];
  float max_map[26];
  float min_map[26];
  int count_map[26];
  long int lines_counted;
} calculations;

struct task {
  long int start;
  long int end;
  long int counter;
  calculations calc;
  char *filename;
  char *stream;
};
int parse_file(FILE *, long int, long int, long int);
void split_chunks(char *, int, int);
void *threaded_parser(void *);
void read_full_file(char *);
struct task *prepare_tasks(char *, int);
void *executeChunkTask(void *);
void execute_tasks(struct task *, int);
void print_report(calculations *);
void onebrc(int, char *);
void mergeCalculations(calculations *, calculations *, calculations *);
void processChunk(struct task *);

#include "types.h"

#define WORKERS 128
#define DATA_FILE "../data.txt"
int main(void) {
  onebrc(WORKERS, DATA_FILE);
  return 1;
}

void onebrc(int workers, char *filepath) {
  struct task *t = prepare_tasks(filepath, workers);
  execute_tasks(t, workers);
}

void execute_tasks(struct task *tasks, int workers) {

  calculations result = {{0.0}, {0.0}, {0.0}, {0}, 0};

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
}

struct task create_task(char *filename, long int start, long int end,
                        char *stream) {
  calculations x = {{0.0}, {0.0}, {0.0}, {0}, 0};
  return (struct task){start, end, 0, x, filename, stream};
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

  result->lines_counted = y->lines_counted + x->lines_counted;
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
      printf(" %c: Min : %.1f, MAX: %.1f, MEAN: %.1f\n", (char)i + 'A',
             x->min_map[i], x->max_map[i], x->sum_map[i] / x->count_map[i]);
      printf("----------------\n");
    }
  }
  printf("Total lines counted : %ld\n", x->lines_counted);
}

void processChunk(struct task *t) {
  char *stream = t->stream;
  char *f = stream + t->start;

  calculations *x = &t->calc;
  int newlineIndex = strcspn(f, "\n");
  char line[16];
  long int counter = 0;
  long int bytes_read = 0;
  int delim_index;
  char *tempS = NULL;
  float temp;
  int cityIndex;

  while (bytes_read < (t->end - t->start)) {
    newlineIndex = strcspn(f, "\n");
    memcpy(line, f, newlineIndex);
    f = f + newlineIndex + 1;
    bytes_read += (newlineIndex + 1);
    delim_index = strcspn(line, ";");
    counter++;
    tempS = &line[0] + delim_index + 1;
    if (tempS != NULL) {
      temp = parse_temperature(tempS);
      cityIndex = f[0] - 'A';
      processCalculations(x, cityIndex, temp);
    }
  }
  x->lines_counted = counter;
  t->counter = counter;
  return;
}

struct task *prepare_tasks(char *filename, int workers) {
  FILE *fp = fopen(filename, "rb");
  fseek(fp, 0L, SEEK_END);
  long int size = ftell(fp);
  rewind(fp);
  long int chunkSize = size / workers;
  long int base = 0L;

  int fd = fileno(fp);
  char *f = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
  if (f == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  struct task *tasks = (struct task *)malloc(sizeof(struct task) * workers);
  int chunkNumber = 0;
  for (;;) {
    long int chunkEnd = base + chunkSize;
    if (chunkEnd >= size) {
      tasks[chunkNumber] = create_task(filename, base, size, f);
      break;
    }

    fseek(fp, chunkEnd, SEEK_SET);
    char buf;
    for (;;) {
      buf = getc(fp);
      if (buf == '\n') {
        break;
      }
      chunkEnd++;
    }

    tasks[chunkNumber] = create_task(filename, base, chunkEnd, f);
    base = chunkEnd + 1;
    chunkNumber++;
  }
  return tasks;
}

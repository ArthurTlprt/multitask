#include <stdio.h>
#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


struct task {
  int file_id;
  int delay;
  struct task * next_task;
};

struct queue {
  struct task * head;
};


struct queue * create_queue () {
  struct queue * m_queue = malloc(sizeof(*m_queue));
  m_queue->head = NULL;
  return m_queue;
}

void push_queue (struct queue *tasks, int file_id, int delay) {

  if(tasks == NULL) {
    exit(EXIT_FAILURE);
  }

  struct task *new_task = malloc(sizeof(*new_task));
  if (new_task == NULL) {
    exit(EXIT_FAILURE);
  }

  new_task->file_id = file_id;
  new_task->delay = delay;


  if(tasks->head == NULL) {
    tasks->head = new_task;
  } else {
    struct task *item = tasks->head;

    while(item->next_task != NULL) {
      item = item->next_task;
    }

    item->next_task = new_task;
  }
}

void read_line(struct queue * tasks, char * line) {

  if(line[0] == 'p') {
    char *pr, *c_file_id, *c_delay;
    const char s[2] = " ";

    pr = strtok(line, s);
    printf("%s\n", pr);
    c_file_id = strtok(NULL, s);
    printf("%s\n", c_file_id);
    c_delay = strtok(NULL, s);
    printf("%s\n", c_delay);

    int file_id = atoi(c_file_id);
    int delay = atoi(c_delay);

    push_queue(tasks, file_id, delay);

  } else if(line[0] == 's') {
    printf("waiting for files...\n");
    char * token = " ";
    const char * s = " ";
    token = strtok(line, s);
    token = strtok(NULL, s);
    sleep(atoi(token));
  }

}


void * read_input(void * arg) {
  struct queue* tasks = (struct queue*) arg;
  char * file_name = "input.txt";
  FILE *f;

  char c;
  f=fopen(file_name,"r");
  if(f == NULL) {
    printf("unable to open the file\n");
    return NULL;
  }

  char line [256];

  while (fgets(line, sizeof(line), f)) {
    read_line(tasks, line);
  }

  fclose(f);
}

int main(int argc, char ** argv) {

  struct queue * tasks = create_queue();

  pthread_t thr_input;
  pthread_create(&thr_input, NULL, read_input, &tasks);
  pthread_join(thr_input, NULL);

  return 0;
}

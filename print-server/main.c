#include <stdio.h>
#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem_files_ready;
pthread_mutex_t tasks_locker = PTHREAD_MUTEX_INITIALIZER;

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

struct task * pop_queue (struct queue * tasks) {
  if(tasks->head == NULL)
    exit(EXIT_FAILURE);

  // je bloque l'édition de la file aux autres thread
  pthread_mutex_lock(&tasks_locker);

  struct task * our_task;

  if(tasks->head->next_task == NULL) {
    our_task = tasks->head;
  //  free(tasks->head);
  } else {
    struct task * new_head = tasks->head->next_task;
    our_task = tasks->head;
  //  free(tasks->head);
    tasks->head = new_head;
  }
  pthread_mutex_unlock(&tasks_locker);
  return our_task;
}

void push_queue (struct queue * tasks, int file_id, int delay) {
  if(tasks == NULL) {
    exit(EXIT_FAILURE);
  }

  // je bloque l'édition de la file aux autres thread
  pthread_mutex_lock(&tasks_locker);
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
  // on permet aux imprimantes de travailler
  sem_post(&sem_files_ready);
  pthread_mutex_unlock(&tasks_locker);

}

struct arg_print_file { struct queue * tasks; int printer_id; };

void * print_file (void * arg) {

  struct arg_print_file * arg_pf = arg;
  struct queue * tasks = arg_pf->tasks;
  int printer_id = arg_pf->printer_id;

  char f_name[12];
  sprintf(f_name, "%d-printer", printer_id);

  for (;;) {
    FILE *f = fopen(f_name, "a");
    sem_wait(&sem_files_ready);

    time_t unix_time;
    unix_time = time(NULL);

    struct task * print_task = pop_queue(tasks);
    char buffer[50] = "";

    sprintf(buffer, "%ld print file %d\n", unix_time, print_task->file_id);
    fprintf(f, "%s", buffer);

    sleep(print_task->delay);
    fclose(f);

    printf("file printed\n");

    free(print_task);
  }

}

void waiting_for_printing (void * arg) {
  struct queue * tasks = arg;
  struct arg_print_file arg_pf0 = { .tasks = tasks, .printer_id = 0 };
  struct arg_print_file arg_pf1 = { .tasks = tasks, .printer_id = 1 };
  struct arg_print_file arg_pf2 = { .tasks = tasks, .printer_id = 2 };

  pthread_t thr_printer1;
  pthread_t thr_printer2;
  pthread_t thr_printer3;

  pthread_create(&thr_printer1, NULL, print_file, &arg_pf0);
  pthread_create(&thr_printer2, NULL, print_file, &arg_pf1);
  pthread_create(&thr_printer3, NULL, print_file, &arg_pf2);

  pthread_join(thr_printer1, NULL);
  pthread_join(thr_printer2, NULL);
  pthread_join(thr_printer3, NULL);

}


void read_line(struct queue * tasks, char * line) {

  if(line[0] == 'p') {
    char *pr, *c_file_id, *c_delay;
    const char s[2] = " ";

    pr = strtok(line, s);
    printf("new file in queue: ");
    c_file_id = strtok(NULL, s);
    printf("%s ", c_file_id);
    c_delay = strtok(NULL, s);
    printf("%s", c_delay);

    int file_id = atoi(c_file_id);
    int delay = atoi(c_delay);
    push_queue(tasks, file_id, delay);

  } else if(line[0] == 's') {
    printf("waiting for files...\n\n");
    char * token = " ";
    const char * s = " ";
    token = strtok(line, s);
    token = strtok(NULL, s);
    sleep(atoi(token));
  }

}


struct arg_read_input { struct queue * tasks; char * file_name;};

void * read_input(void * arg) {

  struct arg_read_input * arg_ri = arg;
  struct queue * tasks = arg_ri->tasks;

  char * file_name = arg_ri->file_name;
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

  if(argc != 2) {
    printf("you need to specify the text file\n");
    return -1;
  }

  // création du buffer
  struct queue * tasks = malloc(sizeof(*tasks));
  struct arg_read_input arg_ri = { .tasks = tasks, .file_name = argv[1] };

  sem_init(&sem_files_ready, 0, 0);
  // on met l'input dans un thread
  // comme ça une attente de fichier ne bloque pas les impressions
  pthread_t thr_input;

  // un thread pour la lecture des requetes d'impression
  pthread_create(&thr_input, NULL, read_input, &arg_ri);
  waiting_for_printing(tasks);

  pthread_join(thr_input, NULL);
//  pthread_join(thr_waiter, NULL);
  return 0;
}

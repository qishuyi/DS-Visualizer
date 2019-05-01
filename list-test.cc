#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// #include "vhelpers.h"

typedef struct node {
  int val;
  struct node* next;
}node_t;

void* head_addr;
const char* myfifo = "myfifo";
const char* request_head_msg = "hello";

void* thread_fn(void* u) {
  // printf("In the thread function\n");
  int fd = open(myfifo, O_RDONLY);
  if (fd == -1) {
    perror("In the tracee, open failed before reading");
    exit(2);
  }
  char buf[50];
  read(fd, buf, sizeof(buf));
  close(fd);

  fd = open(myfifo, O_WRONLY);
  if (fd == -1) {
    perror("In the tracee, open failed before writing");
    exit(2);
  }
  if (strcmp(buf, request_head_msg) == 0) {
    // The tracer is requesting the head address
    size_t bytes_written = write(fd, &head_addr, sizeof(void*));
    if (bytes_written == -1) {
      perror("Write fails in tracee thread_fn");
      exit(2);
    }
  }else {
    // The tracer is requesting the size of a block of memory
    size_t ptr_size = malloc_usable_size(reinterpret_cast<void*>(buf));
    printf("Pointer %p has size %lu\n", reinterpret_cast<void*>(buf), ptr_size);
  }
  close(fd);

  return NULL;
}

int main(int argc, char** argv) {
  node_t* first_node = (node_t*)malloc(sizeof(node_t));
  head_addr = static_cast<void*>(first_node);

  printf("In the tracee: head address is %p\n", head_addr); 
  pthread_t pipe_thread;
  if (pthread_create(&pipe_thread, NULL, thread_fn, NULL) != 0) {
    perror("pthread_create failed");
    exit(2);
  }
  pthread_join(pipe_thread, NULL);



  printf("In the tracee.\nHead allocated at %p\n", first_node);
  // printf("Size of the head is: %lu", malloc_usable_size(first_node));
  first_node->val = 10;

  node_t* second_node = (node_t*)malloc(sizeof(node_t));
  second_node->val = 21;
  first_node->next = second_node;

  return 0;
}


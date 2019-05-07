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
#include <stdbool.h>

#include "vhelpers.h"

typedef struct node {
  int val;
  struct node* next;
}node_t;

void* head_addr;

bool getsize = false;

void* thread_fn(void* u) {
  bool running = true;
  int fd;

  while (running) {
    fd = open(myfifo, O_RDONLY);
    if (fd == -1){
      perror("In the tracee, open failed before reading");
      exit(2);
    }

    pipe_args_t args;

    if (read(fd, &args, sizeof(args)) != sizeof(args)){
      perror("Read fails in tracee thread_fn");
      exit(2);
    }
    close(fd);

    if (args.is_addr == 0 && strcmp(args.msg, request_head_msg) == 0){
      printf("Got a request head message\n");
      // The tracer sends a hello message
      fd = open(myfifo, O_WRONLY);
      if (fd == -1){
	      perror("Open failed in tracee before writing head");
	      exit(2);
      }

      // Create a pipe_args struct with head address
      struct pipe_args pa = {
	      .is_addr = 1,
	      .addr = head_addr
      };

      // Write pipe_args to the tracer
      if (write(fd, &pa, sizeof(pa)) != sizeof(pa)){
	      perror("Write head failed in tracee");
	      exit(2);
      }
    
      close(fd);
    }else {
      // The tracer is requesting the malloced size of an address
      fd = open(myfifo, O_WRONLY);
     
      // Get malloced size of the pointer
      size_t ptr_size = malloc_usable_size(args.addr);

      // Create a struct that contains the pointer size
      struct pipe_args pa = {
	      .ptr_size = ptr_size
      };

      // Send the struct to tracer over pipe
      if (write(fd, &pa, sizeof(pa)) != sizeof(pa)) {
        perror("Write failed in tracee thread_fn");
	      exit(2);
      }
      close(fd);
    }
  }
  return NULL;
}

int main(int argc, char** argv) {
  node_t* first_node = (node_t*)malloc(sizeof(node_t));
  head_addr = static_cast<void*>(first_node);

  pthread_t pipe_thread;
  if (pthread_create(&pipe_thread, NULL, thread_fn, NULL) != 0) {
    perror("pthread_create failed");
    exit(2);
  }
  first_node->val = 10;

  node_t* second_node = (node_t*)malloc(sizeof(node_t));
  second_node->val = 21;
  first_node->next = second_node;

  pthread_join(pipe_thread, NULL);

  return 0;
}


#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>

const char* myfifo = "myfifo";
const char* request_head_msg = "hello";

typedef struct pipe_args {
  int is_addr;
  void* addr;
  char msg[50];
  size_t ptr_size;
}pipe_args_t;

// Thread_fn to be called by the program
void* thread_fn(void* head_addr) {
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

void start_tracing(void* head_addr) {
  pthread_t pipe_thread;
  if (pthread_create(&pipe_thread, NULL, thread_fn, head_addr) == -1) {
    perror("pthread_create fails in tracee");
    exit(2);
  }
}

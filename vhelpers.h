#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 
#include <errno.h>
#include <stdint.h>

#define PAGESIZE 0x1000LL

const char* myfifo = "myfifo";
const char* request_head_msg = "hello";
const char* closing_msg = "done";

typedef struct pipe_args {
  int is_addr;
  void* addr;
  char msg[50];
  size_t ptr_size;
}pipe_args_t;


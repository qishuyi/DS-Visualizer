#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 
#include <errno.h>

#define PAGESIZE 0x1000LL

int register_head(){
  int oflags = O_RDWR;
  mode_t mode = S_IRWXU | S_IRWXG;

  // Create a shared memoy region with the name "head"
  int fd = shm_open("/newregion", oflags, mode);
  if (fd == -1) {
    perror("shm_open failed in tracee");
    exit(2);
  }

  // Truncate the shared memory region to the given size
  if (ftruncate(fd, PAGESIZE)  == -1) {
    perror("ftruncate failed");
    exit(2);
  }



  return fd;
}

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 
#include <errno.h>
#include <stdint.h>

#define PAGESIZE 0x1000LL

/*
 * This function will be called 
 */
int register_head(void* addr){
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

  /*if (write(fd, addr, sizeof(addr)) != sizeof(addr)) {
    perror("Write failed or incomplete");
    exit(2);
  }*/

  //intptr_t dest = (intptr_t)mmap(0, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  //*dest = (intptr_t)addr;

  return fd;
}

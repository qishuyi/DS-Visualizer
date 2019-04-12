#include <stdio.h>
#include <sys/mman.h>

//shm_open, shm_creat

#define PAGE_SIZE 0x1000LL;

// Allign address with the start of a page
#define PG_START(_v) ((_v) & ~(unsigned long)(PAGE_SIZE-1))

unsigned long head_addr = -1;

void* register_head(size_t size){
  int prot_options = PROT_EXEC | PROT_READ | PROT_WRITE;
  int flags = MAP_FIXED | MAP_SHARED;

  // Reserve a page to be overwritten by MAP_FIXED
  unsigned long *reserved_space;
  reserved_space = mmap(0, PAGE_SIZE*2, prot_options, 0, -1, 0);

  // Mmap the head
  void* head = mmap((void*)PG_START(reserved_space), size, prot_options, flags, -1, 0);
  if (head == MAP_FAILED){
    perror("mmap failed");
    exit(2);
  }

  head_addr = (unsigned long)head;

  return head;
}

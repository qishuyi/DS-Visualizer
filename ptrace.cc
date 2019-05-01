#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unordered_map>

#include "vhelpers.h"

typedef struct node {
  int val;
  struct node* next;
}node_t;

typedef struct thread_args {
  //char* file;
  void* addr;
}thread_args_t;

// Head address
void* head = NULL;

pid_t tracee_pid;

// Mode
mode_t mode = S_IRWXU | S_IRWXG;

// A buffer of 0's
char* zerobuf[PAGESIZE] = {0};

const char* myfifo = "myfifo";

std::unordered_map<void*, size_t> ptr_sizes;

void* gethead_thread_fn(void* h) {
  printf("In the get head thread\n");
  // Send message to request the head address
  int fd = open(myfifo, O_WRONLY);
  if (fd == -1) {
    perror("open failed in gethead_thread before writing");
    exit(2);
  }
  const char* msg = "hello";
  int bytes_written = write(fd, msg, sizeof(msg));
  if (bytes_written < 0) {
    perror("write failed in gethead_thread_fn");
    exit(2);
  }
  close(fd);

  printf("Written hello to the pipe\n");
  // Get head address from the pipe
  fd = open(myfifo, O_RDONLY);
  if (fd == -1) {
    perror("open failed in gethead_thread");
    exit(2);
  }
  void* addr;
  int bytes_read = read(fd, &addr, sizeof(void*));
  if (bytes_read < 0) {
    perror("read failed in gethead_thread_fn");
    exit(2);
  }
  head = addr;
  printf("Read %d bytes, Address is %p\n", bytes_read, addr);
  close(fd);

  return NULL;
}

// The thread will open a file and send requests to get a pointer address or get the size of a pointer
void* getsize_thread_fn(void* s) {
  thread_args_t* args = static_cast<thread_args_t*>(s);

  void* addr = args->addr;
	
  // Write the pointer address to the pipe
  int fd = open(myfifo, O_WRONLY);
  if (fd == -1) {
    perror("open failed in getsize_thread_fn before writing");
    exit(2);
  }
  int bytes_written = write(fd, &addr, sizeof(void*));
  if (bytes_written < 0) {
    perror("Write failed in getsize_thread_fn");
    exit(2);
  }
  close(fd);

  // Get the size of the pointer/struct from the pipe
  size_t ptr_size;
  fd = open(myfifo, O_RDONLY);
  if (fd == -1) {
    perror("open failed in getsize_thread_fn before reading");
    exit(2);
  }
  int bytes_read = read(fd, &ptr_size, sizeof(size_t));
  if (bytes_read < 0) {
    perror("Read failed in getsize_thread_fn");
    exit(2);
  }
  printf("In the getsize thread: pointer is at %p, size is %lu\n", addr, ptr_size);
  ptr_sizes[addr] = ptr_size;
  printf("Added the size to map\n");
  close(fd);

  return NULL;
}

int main(int argc, char** argv) {
  // Call fork to create a child process
  pid_t child_pid = fork();
  if(child_pid == -1) {
    perror("fork failed");
    exit(2);
  }

  // If this is the child, ask to be traced
  if(child_pid == 0) {
    if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
      perror("ptrace traceme failed");
      exit(2);
    }

    // Stop the process so the tracer can catch it
    raise(SIGSTOP);

    if(execvp(argv[1], &argv[1])) {
      perror("execvp failed");
      exit(2);
    }

  } else {
    // Wait for the child to stop
    int status;
    int result;
    do {
      result = waitpid(child_pid, &status, 0);
      if(result != child_pid) {
        perror("waitpid failed");
        exit(2);
      }
    } while(!WIFSTOPPED(status));

    // We are now attached to the child process
    printf("Attached!\n");

    // Create a shared memory region to be used by the tracee program later
    /*int fd = shm_open("/newregion", O_RDWR | O_CREAT, mode);
    if (fd == -1) {
      perror("shm_open failed");
      exit(2); 
    }
    printf("mmaped a memory region\n");
    // ftruncate the shared region to size of a page
    if (ftruncate(fd, PAGESIZE) == -1) {
      perror("ftruncate failed");
      exit(2);
    }
    printf("Truncated it to the good size\n");
    // Zero out the shared memory region
    void* head_region = mmap(0, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (head_region == MAP_FAILED) {
      perror("mmap failed");
      exit(2);
    }
    printf("Mmaped the fd at %p\n", head_region);
    memset(head_region, 0, PAGESIZE);
    printf("-1 the region\n");
    // TODO: Not sure if we should close the fd or unmap the region
    close(fd);*/
    //munmap(head_region, PAGESIZE);

    // Create a pipe to communicate with the tracee
    if (mkfifo(myfifo, S_IRWXU) == -1) {
      perror("mkfifo failed in main");
      exit(2);
    }

    printf("Created the fifo\n");
    // Get the head address from the tracee
    pthread_t get_head_thread;
    if (pthread_create(&get_head_thread, NULL, gethead_thread_fn, NULL) != 0) {
      perror("pthread_create failed");
      exit(2);
    }
   
    // Continue the process, delivering the last signal we received (if any)
    if(ptrace(PTRACE_CONT, child_pid, NULL, 0) == -1) {
        perror("ptrace CONT failed");
        exit(2);
    }

    // Wait for the child to stop again
    if(waitpid(child_pid, &status, 0) != child_pid) {
      perror("waitpid failed");
      exit(2);
    }

    // Now repeatedly resume and trace the program
    bool running = true;
    int last_signal = 0;
    while(running) {
      // Continue the process, delivering the last signal we received (if any)
      if(ptrace(PTRACE_SINGLESTEP, child_pid, NULL, last_signal) == -1) {
        perror("ptrace SINGLESTEP failed");
        exit(2);
      }

      // No signal to send yet
      last_signal = 0;

      // Wait for the child to stop again
      if(waitpid(child_pid, &status, 0) != child_pid) {
        perror("waitpid failed");
        exit(2);
      }

      if(WIFEXITED(status)) {
        printf("Child exited with status %d\n", WEXITSTATUS(status));
        running = false;
      } else if(WIFSIGNALED(status)) {
        printf("Child terminated with signal %d\n", WTERMSIG(status));
        running = false;
      } else if(WIFSTOPPED(status)) {
        // Get the signal delivered to the child
        last_signal = WSTOPSIG(status);

        // If the signal was a SIGTRAP, we stopped at a single step
        if (last_signal == SIGTRAP) {
	  if (head) {
  	    long data_read = ptrace(PTRACE_PEEKDATA, child_pid, head, NULL);
  	    if (data_read == -1) {
   	      perror("ptrace peekdata failed in gethead thread");
    	      exit(2);
  	    }
  	    printf("data read: %ld\n", data_read);
	  }
          // Ask the tracee for the size of the allocated head
          pthread_t pipe_thread;
          thread_args_t* args = (thread_args_t*)malloc(sizeof(thread_args_t));
          args->addr = static_cast<void*>(head);
          if (pthread_create(&pipe_thread, NULL, getsize_thread_fn, &args) != 0) {
            perror("The second pthread_create failed");
            exit(2);
          }
          pthread_join(pipe_thread, NULL);
	  printf("Head is at %p, size of head is %lu\n", head, ptr_sizes[head]);

	  //void* addr = head_region;
	  //if (memcmp((void*)head, (void*)zerobuf, sizeof(head)) == 0) {
	    //last_signal = 0;
	    //continue;
	  //}
	  //int val = *((int*)(head));
	  //node_t* next = head+sizeof(int);
	  //printf("Head has been filled with value %p\n", addr);
	  //printf("Next address is %p\n", next);

          last_signal = 0;
        }
      }
    }
    
    return 0;
  }
}


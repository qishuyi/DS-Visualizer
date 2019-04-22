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

#include "vhelpers.h"

typedef struct node {
  int val;
  struct node* next;
}node_t;

long head_allocated;

mode_t mode = S_IRWXU | S_IRWXG;

char* zerobuf[PAGESIZE] = {0};

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

    // TODO: Do some work in the sandboxed child process here
    //       As an example, just run `ls`.
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
    int fd = shm_open("/newregion", O_RDWR | O_CREAT, mode);
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
    printf("Mmaped the fd\n");
    memset(head_region, 0, PAGESIZE);
    printf("-1 the region\n");
    // TODO: Not sure if we should close the fd or unmap the region
    close(fd);
    //munmap(head_region, PAGESIZE);

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
	  node_t* head = (node_t*)head_region;
	  if (memcmp((void*)head, (void*)zerobuf, sizeof(head)) == 0) {
	    close(fd);
	    last_signal = 0;
	    continue;
	  }
	  int val = head->val;
	  printf("Head has been filled with value %d\n", val);

          last_signal = 0;
        }
      }
    }
    
    return 0;
  }
}


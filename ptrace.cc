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

// A struct that packs arguments to a thread function
typedef struct thread_args {
  void* addr;
}thread_args_t;

// Head address
void* head = NULL;
bool head_set = false;


// A boolean to indicate whether we will continue tracing the data structure or not
bool running = true;

// Mode
mode_t mode = S_IRWXU | S_IRWXG;

// A mapping from pointer address to its size
std::unordered_map<void*, size_t> ptr_sizes;

/*
 * Function for getting the head address from tracee
 */
void* gethead_thread_fn(void* h) {
  // Open a file descriptor for writing
  int fd = open(myfifo, O_WRONLY);
  if (fd == -1) {
    perror("open failed in gethead_thread before writing");
    exit(2);
  }

  // Define a struct to send over pipe
  pipe_args_t args;
  args.is_addr = 0;
  strcpy(args.msg, request_head_msg);

  // Send the struct to request for address of the head of data structure
  if (write(fd, &args, sizeof(args)) < 0) {
    perror("write failed in gethead_thread_fn");
    exit(2);
  }

  close(fd);

  // Open another file descriptor for reading
  fd = open(myfifo, O_RDONLY);
  if (fd == -1) {
    perror("open failed in gethead_thread");
    exit(2);
  }

  // Read a struct from pipe
  pipe_args_t answer;

  if (read(fd, &answer, sizeof(pipe_args_t)) != sizeof(pipe_args_t)) {
    perror("read failed in gethead_thread_fn");
    exit(2);
  }

  // Get the head address from the struct
  head = answer.addr;

  head_set = true;

  close(fd);

  // Exit this thread because we only need this function once
  int ret = 0;
  pthread_exit(&ret);

  return NULL;
}

/*
 * The thread function to get the malloced size of a pointer 
 */
void* getsize_thread_fn(void* s) {
  // Unpack thread arguments
  thread_args_t* args = static_cast<thread_args_t*>(s);
  void* addr = args->addr;

  // Write the pointer address to the pipe
  int fd = open(myfifo, O_WRONLY);
  if (fd == -1) {
    perror("open failed in getsize_thread_fn before writing");
    exit(2);
  }

  // printf("Opened fifo in getsize\n");

  // Define a struct as pipe argument
  struct pipe_args pa {
    .is_addr = 1,
    .addr = addr
  };

  // Send the struct over pipe
  if (write(fd, &pa, sizeof(pa)) != sizeof(pa)) {
    perror("Write failed in getsize_thread_fn");
    exit(2);
  }

  close(fd);

  // printf("Written address to the pipe: %p\n", addr);

  // Open another file descriptor for reading
  fd = open(myfifo, O_RDONLY);
  if (fd == -1) {
    perror("open failed in getsize_thread_fn before reading");
    exit(2);
  }

  // Read a struct from pipe
  pipe_args_t answer;
  if (read(fd, &answer, sizeof(answer)) != sizeof(answer)) {
    perror("Read failed in getsize_thread_fn");
    exit(2);
  }

  // printf("In the getsize thread: pointer is at %p, size is %zu\n", addr, answer.ptr_size);

  // Add the malloced size to the global mapping
  ptr_sizes[addr] = answer.ptr_size;

  close(fd);

  return NULL;
}

void* closing_thread_fn(void* args) {
  // Open a file descriptor for writing
  int fd = open(myfifo, O_WRONLY);
  if (fd == -1) {
    perror("open failed in closing_thread_fn");
    exit(2);
  }

  // Create a struct with closing message to send over pipe
  pipe_args_t pa;
  pa.is_addr = 0;
  strcpy(pa.msg, closing_msg);

  // Write the message to pipe
  if (write(fd, &pa, sizeof(pa)) != sizeof(pa)) {
    perror("write failed in closing_thread_fn");
    exit(2);
  } 

  close(fd);

  return NULL;
}

void inspect_memory(int child_pid, void* addr) {
  // If the head is NULL.
  if (!head) {
    printf("Head no longer exists\n");
    return;
  }

  // If the address is NULL, 
  if (!addr) {
    printf("\tNULL\n");
    return;
  }

  // Read data at the given address
  long data_read = ptrace(PTRACE_PEEKDATA, child_pid, addr, NULL);
  if (data_read == -1) {
    // If ptrace peekdata returns -1, it means the address is a value instead of a pointer
    printf("address (%p) contains a value\n", addr);
    return;
  }else {
    printf("data at address (%p) : %p\n", addr, (void*)data_read);

    pthread_t pipe_thread;
    thread_args_t args;
    args.addr = addr;

    // Create a thread to ask tracee for the malloced size of head
    if (pthread_create(&pipe_thread, NULL, getsize_thread_fn, &args) != 0) {
      perror("The second pthread_create failed");
      exit(2);
    }

    // Wait for the thread to exit before accessing the map
    pthread_join(pipe_thread, NULL);

    // Get the size of the mallocaed memory associated with the given pointer
    size_t ptr_size = ptr_sizes[addr];

    printf("Pointer (%p) has size %lu\n", addr, ptr_size);

    void* cur_addr = addr;

    inspect_memory(child_pid, reinterpret_cast<void*>(data_read));

    // Step through the memory region 8 bytes at a time
    for (int i = 0; i < ptr_size / 8 - 1; i++) {
      // Get next address
      cur_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(cur_addr) + 8);

      // Read the data at the next address
      data_read = ptrace(PTRACE_PEEKDATA, child_pid, cur_addr, NULL);

      // Check if data_read is a pointer
      printf("data at address (%p) is %p\n", cur_addr, (void*)data_read);
      
      inspect_memory(child_pid, reinterpret_cast<void*>(data_read));
    }
  }

  printf("\n");
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

    // Create a pipe to communicate with the tracee
    if (mkfifo(myfifo, S_IRWXU) == -1) {
      perror("mkfifo failed in main");
      exit(2);
    }

    // Start a thread to request head address from tracee
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
    int counter = 0;
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

      if(++counter % 100 != 0) continue;

      printf("%d\n", counter / 100);

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

          if (head_set) {
            inspect_memory(child_pid, head);
          }

        }	 

        last_signal = 0;

      }
    }
  }

  return 0;
}



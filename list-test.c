#include <stdio.h>
#include <stdlib.h>

#include "vhelpers.h"

typedef struct node {
  int val;
  struct node* next;
}node_t;

void doStuff(){
  printf("Do stuff\n");
}

int main(int argc, char** argv) {
  int head_fd = register_head();

  node_t* first_node = (node_t*)mmap(0, sizeof(node_t), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, head_fd, 0);

  //node_t* first_node = (node_t*)malloc(sizeof(node_t));
  first_node->val = 10;

  node_t* second_node = (node_t*)malloc(sizeof(node_t));
  second_node->val = 21;
  first_node->next = second_node;

  doStuff();

  printf("head: %d\n", first_node->val);

  //if (shm_unlink("/newregion") == -1){
    //perror("shm_unlink failed");
    //exit(2);
  //}

  return 0;
}


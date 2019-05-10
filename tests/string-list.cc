#include <stdio.h>
#include <stdlib.h>

#include "../vhelpers.h"

typedef struct node {
  char* val;
  struct node* next;
}node_t;


int main(int argc, char** argv) {
  // Set a random seed
  srand(time(NULL));

  // Declare a first node of the linked list
  node_t* first_node = (node_t*)malloc(sizeof(node_t));
  
  start_tracing(static_cast<void*>(first_node));

  // Create 100 nodes in the linked list
  node_t* cur = first_node;
  for (int i = 0; i < 3; i++) {
    // cur->val = rand() % 100;
    char* str = (char*)malloc(sizeof(char));
    cur->val = str;
    cur->next = (node_t*)malloc(sizeof(node_t));
    cur = cur->next;
  }
   
  return 0;
}


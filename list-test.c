#include <stdio.h>
#include "vhelpers.h"

struct node {
  int val;
  struct node* next;
}node_t;


struct list {
  node_t* head;
}btree_t;


int main(int argc, char** argv) {
  node_t* head = (node_t*)register_head(sizeof(node_t));
  
  first_node->val = 0;
  first_node->next = NULL;

  return 0;
}


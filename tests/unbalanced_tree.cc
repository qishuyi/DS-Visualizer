#include "binary_tree.h"
#include <stdio.h>
#include <stdlib.h>

#include "../vhelpers.h"

int main () {
  
  srand(time(NULL));
  binary_tree_t* head = (binary_tree_t*)malloc(sizeof(binary_tree_t));

  start_tracing(static_cast<void*>(head));
 
  binary_tree_t_insert(head, 10);
  binary_tree_t_insert(head, 1);
  binary_tree_t_insert(head, 2);
  binary_tree_t_insert(head, 3);
  binary_tree_t_insert(head, 4);
  binary_tree_t_insert(head, 5);
  binary_tree_t_insert(head, 6);
  binary_tree_t_insert(head, 7);
  binary_tree_t_insert(head, 8);
  binary_tree_t_insert(head, 15);
  binary_tree_t_insert(head, 13);
  binary_tree_t_insert(head, 14);
  binary_tree_t_insert(head, 17);
  binary_tree_t_insert(head, 16);

  binary_tree_t_print(head);
  
  return 0;
}

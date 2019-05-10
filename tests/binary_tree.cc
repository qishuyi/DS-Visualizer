#include "binary_tree.h"
#include <stdio.h>
#include <stdlib.h>

#include "../vhelpers.h"

int main () {
  
  srand(time(NULL));
  binary_tree_t* head = (binary_tree_t*)malloc(sizeof(binary_tree_t));

  start_tracing(static_cast<void*>(head));
 
  for (int i = 0; i < 1000; i++) {
    binary_tree_t_insert(head, rand() % 1000);
  } 
  
  binary_tree_t_print(head);
  
  return 0;
}

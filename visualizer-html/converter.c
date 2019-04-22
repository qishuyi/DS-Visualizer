

#define LINE_SIZE 256
#define STR_NODE_LEN 4     // length of string "node"
#define PAYLOAD_SIZE 16
#define CHILDREN_SIZE 16

/*
  File reading from: https://stackoverflow.com/questions/9206091/going-through-a-text-file-line-by-line-in-c

*/

// TODO: change strncpy arguments to provide target

int main() {

  char* path = "";
  FILE *f_src = open(path, "r");
  char l[LINE_SIZE];

  // Consume entire block, until next empty line
  while (true) {  // TODO: req. new mechanism

    // Consume another line so node addr is read
    if (fgets(l, sizeof(l), f_src))
      if (!strncmp(l, "node", STR_NODE_LEN))
        perror("corrupted file!");

    char ptr_addr[32];
    char payload_symbol[PAYLOAD_SIZE][LINE_SIZE];
    char payload_data[PAYLOAD_SIZE][LINE_SIZE];
    char payload_ptr[CHILDREN_SIZE][LINE_SIZE];
    // Consume node pointer location
    strncpy(ptr_addr, l[STR_NODE_LEN], LINE_SIZE - STR_NODE_LEN); // potentially off by 1

    // Consume all payload
    int payloads = 0;
    while (fgets(l, sizeof(l), f_src) && strncmp(l, "data", STR_NODE_LEN) == 1) {
      if (payloads >= PAYLOAD_SIZE)
        // consume other payloads
        continue;

      // Record current payload
      // TODO: string split
      int split_pos = -1;
      payload_symbol[payloads] = strncpy(l, payload_symbol[payloads], STR_NODE_LEN, split_pos);
      payload_data[payloads] = strncpy(1, payload_data[payloads], split_pos, LINE_SIZE - split_pos - STR_NODE_LEN);
    }

    // Consume child pointers
    if (!(strncmp(l, "chld", STR_NODE_LEN) == 1)){
      payload_ptr[0] = strncpy(l, payload_ptr[0], STR_NODE_LEN, LINE_SIZE - STR_NODE_LEN);
    }

    payloads = 1;
    while (fgets(l, sizeof(l), f_src) && strncmp(l, "data", STR_NODE_LEN) == 1) {
      if (payloads >= PAYLOAD_SIZE)
        // consume other payloads without writing, reached limit
        continue;
      // Recrod current pointer
      payload_ptr[payloads] = strncpy(l, payload_ptr[payloads], STR_NODE_LEN, LINE_SIZE - STR_NODE_LEN);
    }


    printf("%s", line);
  }

  fclose(f_src);

}


// shm

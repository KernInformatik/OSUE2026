#include <stdio.h>

#include "list.h"

int main(int argc, char **argv) {
  // create a linked list containg three elements
  node *head = createHead("first element");
  appendData(head, "second element");
  appendData(head, "third element");

  print(head);

  return 0;
}

#include "list.h"

#include <stdio.h>
#include <stdlib.h>

// creation
node* createHead(char *data) {
  node* head = malloc(sizeof(node)); // acquire memomry for head node
  head->data = data;                // set data pointer
  head->next = NULL;                // set next to NULL to signal it is not valid (this is the last element)
  return head;
}

// helper function, only visible in this file
static node* findLast(node *head) {
  if (head->next == NULL) { // no next node, this must be the last
    return head;
  } else {
    return findLast(head->next); // recursive function call
  }
}

// create new node for data and append to the end of the list
node* append(node* head, char *data) {
  node *newNode = createHead(data);
  node *last = findLast(head);
  last->next = newNode;
  return head;
}

// helper function, only visible in this file
static node* findFirst(node *head) {
  return NULL; // todo
}

node* prepend(node *head, char *data) {
  return NULL; // todo
}

// print, one element per line
static void printPrime(int i, node *head) {
  printf("%d: %s\n", i, head->data);
  if (head->next != NULL) {
    printPrime(i + 1, head->next);
  }
}

void print(node *head) {
  printPrime(0, head);
}

// move
node* forward(node *head) {
  if (head->next != NULL){
    return head->next;
  } else { // return NULL to signal that move is not possible
    return NULL;
  }
}

node* backward(node *head) {
  return NULL; // todo
}

// modify
// remove the given element, merge the list parts and
// return the element before the one we just removed (if it exists)
//     or the element after  the one we just removed (if it exists, but no previous element exists)
//     or NULL (if this was the only elemnt)
node* delete(node *head) {
  return NULL; // todo
}

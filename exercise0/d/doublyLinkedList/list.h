#ifndef __LIST_H__
#define __LIST_H__

typedef struct node_s
{
    char *data;
    struct node_s *next;
    struct node_s *prev;
} DoublyLinkedList;

typedef struct node_l
{
    char *data;
    struct node_l *next;
} LinkedList;

/*This is bad code */
// typedef struct node_s DoublyLinnkedList;

// creation
DoublyLinkedList *createHead(char *data);
DoublyLinkedList *appendData(DoublyLinkedList *head, char *data);
DoublyLinkedList *prepend(DoublyLinkedList *head, char *data);

// print, one element per line
void print(DoublyLinkedList *head);

// move
DoublyLinkedList *forward(DoublyLinkedList *head);
DoublyLinkedList *backward(DoublyLinkedList *head);

// modify
DoublyLinkedList *deleteNode(DoublyLinkedList *head);
void ring(DoublyLinkedList *head);
// static LinkedList *createHeadLinkedList(char *data);
// static LinkedList *appendDataLinkedList(LinkedList *head, char *data);
LinkedList *splitDLLToLinkedList(DoublyLinkedList *head);
LinkedList *findLastNodeLinkedList(LinkedList *head);
void ring(DoublyLinkedList *head);
LinkedList *ringLL(DoublyLinkedList *head);
LinkedList *convertDLL(DoublyLinkedList *head);
LinkedList *splitRing(LinkedList *headRing);
LinkedList *mergeLinkedList(LinkedList *List1, LinkedList *List2);
void destroyLinkedList(LinkedList *head);
void destroyDataLinkedList(LinkedList *head);
#endif // __LIST_H__

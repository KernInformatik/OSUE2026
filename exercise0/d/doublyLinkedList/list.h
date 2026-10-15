#ifndef __LIST_H__
#define __LIST_H__

struct node_s
{
    char *data;
    struct node_s *next;
    struct node_s *prev;
};

/*This is bad code */
typedef struct node_s node;

// creation
node *createHead(char *data);
node *appendData(node *head, char *data);
node *prepend(node *head, char *data);

// print, one element per line
void print(node *head);

// move
node *forward(node *head);
node *backward(node *head);

// modify
node *deleteNode(node *head);

#endif // __LIST_H__

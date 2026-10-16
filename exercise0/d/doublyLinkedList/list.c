#include "list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// creation
DoublyLinkedList *createHead(char *data)
{
    DoublyLinkedList *head = malloc(sizeof(DoublyLinkedList)); // acquire memomry for head DoublyLinkedList
    if (head == NULL)
    {
        fprintf(stderr, "Faile to allocate memory for head");
        exit(EXIT_FAILURE);
    }

    head->data = data; // set data pointer
    head->next = NULL; // set next to NULL to signal it is not valid (this is the
                       // last element)
    head->prev = NULL;
    return head;
}

// helper function, only visible in this file
static DoublyLinkedList *findLast(DoublyLinkedList *head)
{
    if (head->next == NULL)
    { // no next DoublyLinkedList, this must be the last
        return head;
    }
    else
    {
        return findLast(head->next); // recursive function call
    }
}

/**
 * I thought this was perfectly working code, but nah this shit had also to be fixed fuck that
 */
// create new DoublyLinkedList for data and append to the end of the list
DoublyLinkedList *appendData(DoublyLinkedList *head, char *data)
{
    DoublyLinkedList *newDoublyLinkedList = createHead(data);
    DoublyLinkedList *last = findLast(head);
    last->next = newDoublyLinkedList;
    newDoublyLinkedList->prev = last;
    newDoublyLinkedList->next = NULL;
    // return head;
    return newDoublyLinkedList;
}

// helper function, only visible in this file
static DoublyLinkedList *findFirst(DoublyLinkedList *head)
{
    if (head->prev == NULL)
    {
        return head;
    }
    else
    {
        return findFirst(head->prev);
    }
}

/* Adding a new DoublyLinkedList to the first position*/
DoublyLinkedList *prepend(DoublyLinkedList *head, char *data)
{
    DoublyLinkedList *first = findFirst(head);
    appendData(first, data);
    return first;
}

// print, one element per line
static void printPrime(int i, DoublyLinkedList *head)
{
    // printf("%d: %s\n", i, head->data);
    // if (head->next != NULL)
    // {
    //     printPrime(i + 1, head->next);
    // }

    for (DoublyLinkedList *c = head; c != NULL; c = c->next)
    {
        printf("%d: %s\n", i++, c->data);
    }
}

void print(DoublyLinkedList *head)
{
    printPrime(0, head);
}

// move
DoublyLinkedList *forward(DoublyLinkedList *head)
{
    if (head->next != NULL)
    {
        return head->next;
    }
    else
    { // return NULL to signal that move is not possible
        return NULL;
    }
}

DoublyLinkedList *backward(DoublyLinkedList *head)
{
    if (head->prev != NULL)
    {
        return head->prev;
    }
    else
    {
        return NULL;
    }
}

// modify
// remove the given element, merge the list parts and
// return the element before the one we just removed (if it exists)
//     or the element after  the one we just removed (if it exists, but no
//     previous element exists) or NULL (if this was the only elemnt)
DoublyLinkedList *deleteNode(DoublyLinkedList *head)
{
    if (head == NULL)
    {
        fprintf(stderr, "Cannot delete DoublyLinkedList, head is empty or bad memory?");
        exit(EXIT_FAILURE);
    }

    DoublyLinkedList *DoublyLinkedListPrev = NULL;
    DoublyLinkedList *DoublyLinkedListNext = NULL;

    DoublyLinkedListNext = head->next;

    DoublyLinkedListPrev = head->prev;

    if (DoublyLinkedListNext != NULL)
        DoublyLinkedListNext->prev = DoublyLinkedListPrev;

    if (DoublyLinkedListPrev != NULL)
        DoublyLinkedListPrev->next = DoublyLinkedListNext;

    printf("The deleted data was: %s in %s\n", head->data, __func__);

    free(head);

    DoublyLinkedList *rv = (DoublyLinkedListPrev == NULL ? DoublyLinkedListNext : DoublyLinkedListPrev);
    return rv;
}

/**
 * @brief Linking the first and the last DoublyLinkedList together
 *
 * @param head
 */
void ring(DoublyLinkedList *head)
{
    assert(head != NULL);
    DoublyLinkedList *last = NULL;
    DoublyLinkedList *first = NULL;

    last = findLast(head);
    first = findFirst(head);

    last->next = first;
    first->prev = last;
}

static inline LinkedList *createHeadLinkedList(char *data)
{
    LinkedList *headLinkedList = (LinkedList *)malloc(sizeof(LinkedList));

    if (headLinkedList == NULL)
    {
        fprintf(stderr, "failed to allcoate memory");
        return NULL;
    }
    headLinkedList->data = data;
    headLinkedList->next = NULL;

    return headLinkedList;
}

static inline LinkedList *appendDataLinkedList(LinkedList *head, char *data)
{
    LinkedList *newNode = createHeadLinkedList(data);
    head->next = newNode;
    return newNode;
}

/**
 * @brief splits a DLL into a  normal list LinkedList
 *
 * @param head
 * @return the head of the Linked list!
 */
LinkedList *splitDLLToLinkedList(DoublyLinkedList *head)
{
    DoublyLinkedList *first = findFirst(head);

    LinkedList *headLL = createHeadLinkedList(first->data);
    LinkedList *append = headLL;

    for (DoublyLinkedList *c = first->next; c != NULL; c = c->next)
    {
        LinkedList *newEntry = createHeadLinkedList(c->data);
        append->next = newEntry;
        append = newEntry;

        if (c == first)
        {
            break;
        }
    }

    return headLL; // returns the head of the linked list
}

LinkedList *findLastNodeLinkedList(LinkedList *head)
{
    if (head == NULL)
    {
        return NULL;
    }
    LinkedList *rv = NULL;

    while (head->next != NULL)
    {
        rv = (head = head->next);
    }
    return rv;
}

LinkedList *ringLL(DoublyLinkedList *head)
{
    assert(head != NULL);
    LinkedList *headLL = splitDLLToLinkedList(head);
    LinkedList *last = findLastNodeLinkedList(headLL);
    last->next = headLL;
    return headLL;
}

LinkedList *convertDLL(DoublyLinkedList *head)
{
    LinkedList *rv = ringLL(head);
    return rv;
}
/*Assume headRing is a converted DLL */
LinkedList *splitRing(LinkedList *headRing)
{
    LinkedList *firstNode = findLastNodeLinkedList(headRing)->next;

    LinkedList *rv = createHeadLinkedList(firstNode->data);
    LinkedList *append = rv;

    for (LinkedList *c = firstNode; c->next != NULL; c = c->next)
    {
        LinkedList *newEntry = createHeadLinkedList(c->data);
        append->next = newEntry;
        append = newEntry;
    }

    return rv;
}

void destroyLinkedList(LinkedList *head)
{
    LinkedList *firstNode = findLastNodeLinkedList(head)->next;

    for (LinkedList *c = firstNode; c->next != findLastNodeLinkedList(head); c = c->next)
    {
        LinkedList *node = firstNode;
        free(node);
    }
}

void destroyDataLinkedList(LinkedList *head)
{
    LinkedList *firstNode = findLastNodeLinkedList(head)->next;

    for (LinkedList *c = firstNode; c->next != findLastNodeLinkedList(head); c = c->next)
    {
        LinkedList *node = firstNode;
        free(node);
    }
    destroyLinkedList(head);
}
/**
 * @brief merges two lists together, appends List2 to List1
 *
 * @param List1
 * @param List2
 * @return LinkedList*
 */

LinkedList *mergeLinkedList(LinkedList *List1, LinkedList *List2)
{
    assert(List1 != NULL);
    assert(List2 != NULL);

    if (findLastNodeLinkedList(List1)->next != NULL)
    {
        splitRing(findLastNodeLinkedList(List1)->next);
    }

    if (findLastNodeLinkedList(List2)->next != NULL)
    {
        splitRing(findLastNodeLinkedList(List2)->next);
    }

    LinkedList *rv = createHeadLinkedList(List1->data);

    LinkedList *tmp = List1;

    while (tmp != NULL)
    {
        if (tmp->next == NULL)
        {
            tmp = List2;
        }
        appendDataLinkedList(rv, tmp->data);
        tmp = tmp->next;
    }

    return rv;
}

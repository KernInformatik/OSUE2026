#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "list.h"

int main(void)
{
    // Test 1: DoublyLinkedList
    DoublyLinkedList *head = createHead("A");
    DoublyLinkedList *b = appendData(head, "B");
    DoublyLinkedList *c = appendData(head, "C");

    assert(head->next == b);
    assert(b->prev == head);
    assert(c->prev == b);
    assert(forward(head) == b);
    assert(backward(c) == b);

    // Test 2: Delete
    // assert(deleteNode(b) == head);
    // assert(head->next == c);
    // assert(c->prev == head);

    // deleteNode(c);
    // deleteNode(head);

    // Test 3: Split DLL to LinkedList

    LinkedList *list = splitDLLToLinkedList(head);
    printf("%s", list->data);
    assert(strcmp(list->data, "A") == 0);
    printf("%s", list->next->next->data);
    assert(strcmp(list->next->data, "B") == 0);
    printf("%s", list->next->next->data);
    assert(strcmp(list->next->next->data, "C") == 0);
    assert(list->next->next->next == NULL);

    // Test 4: Find last node
    LinkedList *last = findLastNodeLinkedList(list);

    assert(strcmp(last->data, "C") == 0);
    assert(last->next == NULL);

    // Test 5: Ring DLL
    DoublyLinkedList *ringHead = createHead("A");
    DoublyLinkedList *ringB = appendData(ringHead, "B");
    DoublyLinkedList *ringC = appendData(ringHead, "C");

    ring(ringHead);

    assert(ringC->next == ringHead);
    assert(ringHead->prev == ringC);
    assert(ringB->next == ringC);

    // Test 6: ringLL
    DoublyLinkedList *head2 = createHead("A");
    appendData(head2, "B");
    appendData(head2, "C");

    LinkedList *ringList = ringLL(head2);

    assert(strcmp(ringList->data, "A") == 0);
    assert(strcmp(ringList->next->data, "B") == 0);
    assert(strcmp(ringList->next->next->data, "C") == 0);
    assert(ringList->next->next->next == ringList);

    // Test 7: convertDLL
    DoublyLinkedList *head3 = createHead("X");
    appendData(head3, "Y");

    LinkedList *converted = convertDLL(head3);

    assert(strcmp(converted->data, "X") == 0);
    assert(strcmp(converted->next->data, "Y") == 0);
    assert(converted->next->next == converted);

    // Test 8: splitRing
    LinkedList *split = splitRing(ringList);

    assert(strcmp(split->data, "A") == 0);
    assert(strcmp(split->next->data, "B") == 0);
    assert(strcmp(split->next->next->data, "C") == 0);

    // Test 9: mergeLinkedList
    DoublyLinkedList *head4 = createHead("A");
    appendData(head4, "B");

    DoublyLinkedList *head5 = createHead("C");
    appendData(head5, "D");

    LinkedList *list1 = splitDLLToLinkedList(head4);
    LinkedList *list2 = splitDLLToLinkedList(head5);

    LinkedList *merged = mergeLinkedList(list1, list2);

    assert(merged != NULL);
    assert(strcmp(merged->data, "A") == 0);

    puts("Tests completed.");

    return 0;
}
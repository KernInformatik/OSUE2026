This small program implements a linked list. So far it is only "singly-linked", that is, linked in one direction (from one element to the next).

Here are a few small exercises. Try as many as you like, in any order that works for you:
You may even skip 3. and implement the remaining exercises for a singly-linked list.

1. Run make and then run the program:

    make && ./main

2. Read the program to understand what it does.

3. Finish the implementation and turn it into a doubly-linked list, that is, a list where each element has a pointer to the next *and* the previous element.
   Update all functions where it is necessary.

4. Also implement the following functions (which are present as dummies)

     node* backward(node *head); // is this possible for a singly-linked list?
     node* prepend(node *head, char *data);
     node* delete(node *head);

5. Look at the Makefile. Change it so it uses one variable for the compiler and one variable for the compiler flags.
6. Also in the Makefile: Replace the targets "list.o" and "main.o" with a single new target using patterns.
7. Also in the Makefile: Add a new target "clean" that removes all object files (ending in .o) and the file main

8. Rewrite static void printPrime(int i, node *head) so it does the same, but is not a recursive function.
9. Add a function that ties the first and last node of a linked list together such that it becomes a ring, or an "infinite list".
10. Add a function that can split a ring into one regular list with a beginning and an end without losing any elements.
11. Make a list into a ring and split it again, such that the resulting list has a different head node than the original.

12. Write a function that destroys a linked list, freeing up all the memory it used for nodes (but don't worry about the data strings)
13. Write another function that destroys a linked list, but also frees up the memory used by the data strings. (be careful to only use strings you can actually free when testing this)

14. Write a function to append a whole list to another list.

15. Modify main such that it takes the command line arguments passed to the program and creates a linked list from them.
#define _POSIX_C_SOURCE 200809L /* must be before ALL includes (needed for getline/strdup with -std=c99) */

#include "list.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#define error(msg, program_name)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("%s\n", msg);                                                                                           \
        fprintf(stderr, "Usage: %s [-o outfile] [elements...]\n", program_name);                                       \
        printf("Error (%d): %s\n", errno, strerror(errno));                                                            \
        exit(EXIT_FAILURE);                                                                                            \
    } while (0)

typedef struct IO
{
    FILE *INPUT;
    FILE *OUTPUT;
} IO;

int main(int argc, char **argv)
{
    IO control = {.INPUT = stdin, .OUTPUT = stdout};
    DoublyLinkedList *head = NULL, *tail = NULL; /* must start as NULL */

    int option;
    bool optionInput = false;

    while ((option = getopt(argc, argv, "o:f")) != -1)
    {
        switch (option)
        {
        case 'o':
            // for good measure i guess
            if (freopen(optarg, "w", stdout) == NULL)
            {
                perror(optarg);
                exit(EXIT_FAILURE);
            }
            break;
        case 'f':
            optionInput = true;
            break;
        default:
            error("an unexpected error occured", "mylist");
            break;
        }
    }

    do
    {
        if (optionInput && (argc - optind) > 0)
        {
            control.INPUT = fopen(argv[optind], "r");
            if (control.INPUT == NULL)
            {
                perror(argv[optind]);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            control.INPUT = stdin;
        }

        char *line = NULL;
        size_t line_buffer_length = 0;
        ssize_t read_size = 0;

        while ((read_size = getline(&line, &line_buffer_length, control.INPUT)) != -1)
        {
            if (line[read_size - 1] == '\n')
            {
                line[read_size - 1] = '\0';
            }

            char *copy = strdup(line);
            assert(copy != NULL);

            if (head == NULL)
            {
                head = tail = createHead(copy);
            }
            else
            {
                tail = appendData(tail, copy);
            }
        }

        fprintf(control.OUTPUT, "\n");
        if (control.INPUT != stdin)
        {
            fclose(control.INPUT);
        }
        free(line);
        line = NULL;
        line_buffer_length = 0;
        fflush(control.OUTPUT);

    } while (++optind < argc);

    if (head != NULL)
    {
        print(head);

        LinkedList *result = splitDLLToLinkedList(head);

        /* both lists share the same strings: free them exactly once */
        destroyDataLinkedList(result);

        while (head != NULL) /* DLL nodes only */
        {
            DoublyLinkedList *next = head->next;
            free(head);
            head = next;
        }
    }

    fclose(control.OUTPUT);
    exit(EXIT_SUCCESS);
}
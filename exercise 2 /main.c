/**
 * @file main.c
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief
 * @version 0.1
 * @date 2026-11-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "common.h"

typedef struct Point
{
    float x;
    float y;
    char *line
} Point;

void usage()
{
    printf("quadtree <goal> <distance> <min_x> <max_x> <min_y> <max_y>\n");
    error_exit("Wrong Input...Exiting\n");
}

static inline int countLines(char *s)
{
    if (s == NULL)
    {
        error_exit("Invalid input, exiting!");
    }
    int c = 0, nl = 0;

    while ((c = getchar()) != EOF)
    {
        if (c == '\n')
        {
            nl++;
        }
    }
    c = 0;
    return nl;
}

float parseInput(int argc, char **argv)
{
    return 0.0;
}

int main(int argc, char **argv)
{
    char *line = NULL;
    size_t size = 0;
    ssize_t nread;

    if (argc != 2)
    {
        size = 0;
        nread = 0;
        free(line);
        fflush(stdout);
        usage();
        fprintf(stderr, "Error occured");
        exit(EXIT_FAILURE);
    }

    while ((nread = getline(&line, &size, fopen(argv[1], "r"))) != -1)
    {
    }

    size = 0;
    nread = 0;
    free(line);
    exit(EXIT_SUCCESS);
}

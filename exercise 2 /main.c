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

    return nl;
}

float parseInput(int argc, char **argv)
{
    return 0.0;
}

int main(int argc, char **argv)
{
    exit(EXIT_SUCCESS);
}

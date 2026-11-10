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
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>

void usage()
{
    printf("./quadtree <goal> <distance> <min_x> <max_x> <min_y> <max_y>\n");
}

/**
 * @brief counts the number of lines in a given file
 *
 * @param s
 * @return int
 */
static inline int countLines(FILE *s)
{
    if (s == NULL || s != stdin)
    {
        fclose(s);
        error_exit("Invalid input or no input given");
    }
    int rv = 0;

    while (!feof(s))
    {
        int c = fgetc(s);
        if (c == '\n')
        {
            rv++;
        }
    }

    return rv + 1;
}

double distanceToGoal(float x, float y, float goal_x, float goal_y)
{
    double dx = (double)x - goal_x;
    double dy = (double)y - goal_y;
    return sqrt(dx * dx + dy * dy);
}

/**
 * @brief main function, on here the main part begins for obvious reasons
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv)
{

    if (argc != 7)
    {
        usage();
        error_exit("Not enough arguments, exiting");
    }

    struct pollfd fds;
    int check = 0;
    fds.fd = 0;
    fds.events = POLLIN;
    check = poll(&fds, 1, 0);

    if (check != 1)
    {
        printf("nothing to do, no points given... exiting");
        exit(EXIT_SUCCESS);
    }

    float goal_x = 0, goal_y = 0, min_x = 0, max_x = 0, max_y = 0, min_y = 0, distance = 0;

    char *goal = strdup(argv[1]);
    char *end1 = NULL, *end2 = NULL;

    goal_x = strtof(goal, &end1);
    goal_y = strtof(end1 + 1, &end2);

    if (goal == end1 || end1 + 1 == end2 || *end1 == ',' || *end2 != '\0' || goal_x > FLT_MAX || goal_y > FLT_MAX ||
        errno == ERANGE)
    {
        free(goal);
        usage();
        error_exit("Couldn't parse values from input");
    }

    distance = strtof(argv[2], NULL);
    min_x = strtof(argv[3], NULL);
    max_x = strtof(argv[4], NULL);
    min_y = strtof(argv[5], NULL);
    max_y = strtof(argv[6], NULL);

    float mid_x = (min_x + max_x) / 2;
    float mid_y = (min_y + max_y) / 2;

    printf("Parsed input:\n%.2f %.2f %.2f %.2f %.2f %.2f %.2f \nmidx: %f\nmidy: %f", goal_x, goal_y, distance, min_x,
           max_x, min_y, max_y, mid_x, mid_y);

    printf("%s\n", goal);
    free(goal);

    int n = countLines(stdin);
    if (fseek(stdin, 0, SEEK_SET) != 0)
    {
        error_exit("stdin is not seekable (piped input?)");
    }
    printf("argc: %d\nlines: %d\n", argc, n);
    float input_x = 0, input_y = 0;

    if (n == 1)
    {

        char *line = NULL;
        size_t size = 0;
        ssize_t nread = 0;
        char *endPtr;
        while ((nread = getline(&line, &size, stdin)) != -1)
        {
            char *tmp = strdup(line);
            // input_x = strtof(strtok_r(tmp, ",", &end), NULL);
            // input_y = strtof(strtok_r(NULL, ",", &end), NULL);
            input_x = strtof(tmp, &endPtr);
            input_y = strtof(endPtr + 1, NULL);
            if (tmp == endPtr || *endPtr != '\0' || input_x > FLT_MAX || input_y > FLT_MAX)
            {
                free(tmp);
                usage();
                error_exit("Couldn't parse values from input");
            }

            // input_x = strtof(tmp, &end);
            // input_y = strtof(end + 1, NULL);
            free(tmp);
        }

        float n = distanceToGoal(input_x, input_y, goal_x, goal_y);
        printf("distance: %f\ninputx: %f\ninputy: %f\n", n, input_x, input_y);

        if (n <= distance)
        {
            printf("%.3f: %s\n", n, line);
        }
        free(line);
        n = 0;
    }

    /*Fork recursion begins here*/
    if (n > 1)
    {
    }

    exit(EXIT_SUCCESS);
}

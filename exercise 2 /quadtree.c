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
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>

/**
 * @brief
 *
 */
void usage()
{
    fprintf(stderr, "./quadtree <goal> <distance> <min_x> <max_x> <min_y> <max_y>\n");
}
/**
 * @brief
 *
 */
typedef struct
{
    pid_t pid;
    FILE *to;
    FILE *from;
} child_t;

/**
 * @brief
 *
 * @param x
 * @param y
 * @param mid_x
 * @param mid_y
 * @return int
 */
static int quadrant(float x, float y, float mid_x, float mid_y)
{ /* 0 BL, 1 BR, 2 TL, 3 TR */
    return (x >= mid_x ? 1 : 0) + (y >= mid_y ? 2 : 0);
}

/**
 * @brief
 *
 * @param x
 * @param y
 * @param goal_x
 * @param goal_y
 * @return double
 */
double distanceToGoal(float x, float y, float goal_x, float goal_y)
{
    double dx = (double)x - goal_x;
    double dy = (double)y - goal_y;
    return sqrt(dx * dx + dy * dy);
}

/**
 * @brief
 *
 * @param s
 * @param x
 * @param y
 * @return int
 */
int parsePoint(char const *s, float *x, float *y)
{
    char *goal = strdup(s);
    char *end1 = NULL, *end2 = NULL;

    *x = strtof(goal, &end1);
    *y = strtof(end1 + 1, &end2);

    if (end1 == goal || *end1 != ',' || *end2 != '\0' || end2 == end1 + 1 || errno == ERANGE || !isfinite(*y) ||
        !isfinite(*x))
    {
        fprintf(stderr, "pid %d: bad point [%s]\n", (int)getpid(), s);
        free(goal);
        usage();
        error_exit("Couldn't parse values from input, maybe use a valid goal?");
    }
    free(goal);

    return 0;
}

/**
 * @brief
 *
 * @param children
 * @param i
 * @param program_name
 * @param goal_string
 * @param distance
 * @param values
 * @return int
 */
static int startChildren_Processes(child_t children[4], int i, char *program_name, char *goal_string, char *distance,
                                   float values[4])
{
    int in[2], out[2]; /*p-c | c - p*/

    if (pipe(in) == -1 || pipe(out) == -1)
    {
        free(program_name);
        free(goal_string);
        i = 0;
        fprintf(stderr, "Failed to establish pipes...exiting\n");
        return -1;
    }

    fflush(stdout);
    fflush(stderr);

    pid_t pid = fork();

    /**
     * @brief Construct a new switch object
     * parent --in[1] ====> in[0]--  child's stdin
     * parent <--out[0] ==== out[1]-- child's stdout
     */
    switch (pid)
    {
    case -1:
        free(program_name);
        free(goal_string);
        i = 0;
        // fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1;
        break;
    case 0:
        // fprintf(stderr, "Child PID is %jd\n", (intmax_t)getpid());
        /*redirects the child pipes to stdin and stdout */

        close(in[1]);
        close(out[0]);
        dup2(in[0], STDIN_FILENO);
        dup2(out[1], STDOUT_FILENO);
        close(in[0]);
        close(out[1]);
        /*close all the FILES from the children */
        for (int k = 0; k < i; ++k)
        {
            fclose(children[k].to);
            fclose(children[k].from);
        }
        /*main part write the values in a buffer, execute the values from the buffer with execlp*/
        /* We are in the child process (after fork). It inherited copies of the
         * pipe FILEs of all siblings started before it (children[0..i-1]).
         * Close them here: if this child kept a write end of a sibling's
         * stdin pipe open, that sibling would never see EOF when the parent
         * closes its own end, and the program would hang. */
        for (int k = 0; k < i; ++k)
        {
            fclose(children[k].to);   /* write end towards sibling k */
            fclose(children[k].from); /* read end from sibling k     */
        }

        /* exec can only pass strings as arguments, so convert the four
         * floats of this quadrant {x_min, x_max, y_min, y_max} to text.
         * "%.9g" (9 significant digits) is enough for a float to survive
         * the round trip through text exactly. */
        char s[4][128];
        for (int k = 0; k < 4; k++)
        {
            snprintf(s[k], sizeof(s[k]), "%.9g", values[k]);
        }

        /* This is the recursion: replace this child with a fresh instance of
         * the same program, which now only searches inside its quadrant
         * (given by s[0..3]). */
        execlp(program_name, program_name, goal_string, distance, s[0], s[1], s[2], s[3], (char *)NULL);

        /* Only reached if execlp failed, on success it never returns. */
        error_exit("Failed to exec\n");
        break;
        break;
    default:
        /*closes the pipes redirect the events to the child*/
        close(in[0]);
        close(out[1]);
        children[i].pid = pid;
        children[i].to = fdopen(in[1], "w");
        children[i].from = fdopen(out[0], "r");
        return (children[i].to && children[i].from) ? 0 : -1;
        break;
    }

    return 0;
}

/**
 * @brief
 *
 * @param children
 * @return int
 */
static int killChildren_Processes(child_t children[4])
{
    for (int i = 0; i < 4; ++i)
    {
        fclose(children[i].to);
    }

    char *line_buf = NULL;
    size_t size = 0;
    for (int i = 0; i < 4; ++i)
    {
        while ((getline(&line_buf, &size, children[i].from)) != -1)
        {
            fputs(line_buf, stdout);
        }
    }
    free(line_buf);
    for (int i = 0; i < 4; i++)
    {
        int status;
        if (WIFEXITED(status))
        {
            // printf("exited, status=%d\n", WEXITSTATUS(status));
        }
        if (waitpid(children[i].pid, &status, WNOHANG) != 0 || WEXITSTATUS(status) != EXIT_SUCCESS ||
            !WIFEXITED(status) || !WIFSIGNALED(status))
        {
            // fprintf(stderr, "Couldnt terminate child processes: %s\n", strerror(errno));
            return -1;
        }
    }
    return 0;
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

    errno = 0;

    float goal_x = 0, goal_y = 0, min_x = 0, max_x = 0, max_y = 0, min_y = 0, distance = 0;

    char *goal = strdup(argv[1]);
    char *end1 = NULL, *end2 = NULL;

    goal_x = strtof(goal, &end1);
    goal_y = strtof(end1 + 1, &end2);

    if (end1 == goal || *end1 != ',' || *end2 != '\0' || end2 == end1 + 1 || errno == ERANGE || !isfinite(goal_x) ||
        !isfinite(goal_y))
    {
        free(goal);
        usage();
        error_exit("Couldn't parse values from input, maybe use a valid goal?");
    }

    distance = strtof(argv[2], NULL);
    min_x = strtof(argv[3], NULL);
    max_x = strtof(argv[4], NULL);
    min_y = strtof(argv[5], NULL);
    max_y = strtof(argv[6], NULL);

    float mid_x = (min_x + max_x) / 2;
    float mid_y = (min_y + max_y) / 2;

    // fprintf(stdout,
    //       "Parsed input:\n%.2f %.2f %.2f %.2f %.2f %.2f %.2f \nmidx: %f\nmidy: "
    //       "%f\n",
    //       goal_x, goal_y, distance, min_x, max_x, min_y, max_y, mid_x, mid_y);

    free(goal);

    /*Recursion begins here */
    float values[4][4] = {
        {min_x, mid_x, min_y, mid_y}, /* 0 bottom left  */
        {mid_x, max_x, min_y, mid_y}, /* 1 bottom right */
        {min_x, mid_x, mid_y, max_y}, /* 2 top    left  */
        {mid_x, max_x, mid_y, max_y}  /* 3 top    right */
    };

    child_t children[4];

    char *line = NULL, *first = NULL;
    size_t size = 0;
    ssize_t nread;
    float x, y, first_line_x = 0, first_line_y = 0;
    int startedChildren = 0;

    while ((nread = getline(&line, &size, stdin)) != -1)
    {
        line[strcspn(line, "\n")] = '\0';

        // printf("current line read:%s\n", line);

        if (parsePoint(line, &x, &y) != 0)
        {
            // fprintf(stderr, "pid %d: bad goal [%s]\n", (int)getpid(), argv[1]);
            free(line);
            usage();
            error_exit("Couldn't parse values from input, maybe use a valid goal?");
        }

        if (first == NULL)
        {
            first = strdup(line);
            first_line_x = x;
            first_line_y = y;
            continue;
        }

        if (strcmp(first, line) == 0)
        {
            continue;
        }

        /* Children are started lazily: only when a second point arrives.
         * Until then, the first point is only buffered in `first`. */
        if (!startedChildren)
        {
            // fprintf(stdout, "%.3f: %s\n", distance, first);

            // fprintf(stderr, "reached child start\n");

            /* Make sure the children are only started once */
            startedChildren = 1;

            /* Start one child per quadrant (0 = bottom left, 1 = bottom right,
             * 2 = top left, 3 = top right). values[i] is the rectangle
             * {x_min, x_max, y_min, y_max} of quadrant i. The child gets it
             * as its new search area and runs this same program again. */
            for (int i = 0; i < 4; i++)
            {
                if (startChildren_Processes(children, i, argv[0], argv[1], argv[2], values[i]) != 0)
                {
                    error_exit("Error starting the child processes...exiting\n");
                }
            }

            /* The first point was only buffered so far, so now it has to be
             * forwarded to the child whose quadrant contains it. */
            fprintf(children[quadrant(first_line_x, first_line_y, mid_x, mid_y)].to, "%s\n", first);
        }

        /* Forward the current point through the pipe to the child that is
         * responsible for its quadrant. This runs for every following point. */
        fprintf(children[quadrant(x, y, mid_x, mid_y)].to, "%s\n", line);
    }
    // print and use accordingly the quadrants
    free(line);

    if (!startedChildren)
    {
        if (first != NULL)
        {
            double d = distanceToGoal(first_line_x, first_line_y, goal_x, goal_y);
            if (d <= distance)
            {
                fprintf(stdout, "%.3f: %s\n", d, first);
            }
        }
    }
    else
    {
        killChildren_Processes(children);
    }

    free(first);

    exit(EXIT_SUCCESS);
}

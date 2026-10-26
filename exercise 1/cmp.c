/**
 * @file generator.c
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief
 * @version 0.2
 * @date 2026-10-20
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "__semaphore_interface.h"
#include <ctype.h>
#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Parses "u-v" arguments into a heap allocated edge list.
 *        The caller must free(result.NumberOfEdges).
 */
static struct EdgeList parseInput(int argc, char **argv)
{
    struct EdgeList rv = {0};
    size_t n = (argc > 1) ? (size_t)(argc - 1) : 0;

    if (n < 1)
    {
        error_exit("too few arguments");
    }
    if (n > 1028)
    {
        error_exit("too many arguments");
    }

    rv.NumberOfEdges = malloc(n * sizeof *rv.NumberOfEdges);
    if (rv.NumberOfEdges == NULL)
    {
        error_exit("malloc failed");
    }

    for (size_t i = 0; i < n; i++)
    {
        const char *arg = argv[i + 1];

        for (size_t j = i + 1; j < n; j++)
        {
            if (strcmp(arg, argv[j + 1]) == 0)
            {
                error_exit("duplicate arguments");
            }
        }

        if (!isdigit((unsigned char)arg[0]))
        {
            error_exit("missing or non-digit node in argument");
        }

        char *end = NULL;
        unsigned long u = strtoul(arg, &end, 10);
        if (*end != '-')
        {
            error_exit("no dash in argument");
        }

        const char *rest = end + 1;
        if (!isdigit((unsigned char)rest[0]))
        {
            error_exit("missing or non-digit node in argument");
        }

        unsigned long v = strtoul(rest, &end, 10);
        if (*end != '\0')
        {
            error_exit("invalid characters in argument");
        }

        if (u > UINT_MAX || v > UINT_MAX)
        {
            error_exit("node number too large");
        }

        rv.NumberOfEdges[i].u.node_number = (unsigned int)u;
        rv.NumberOfEdges[i].v.node_number = (unsigned int)v;
    }

    rv.length = (ssize_t)n;
    return rv;
}

/** @brief Adds val to the vertex array if it is not already in there. */
static void addVertex(unsigned int *vertices, size_t *count, unsigned int val)
{
    for (size_t i = 0; i < *count; i++)
    {
        if (vertices[i] == val)
        {
            return;
        }
    }
    vertices[(*count)++] = val;
}

/** @brief Fisher-Yates shuffle. */
static void shuffleVertices(unsigned int *vertices, size_t length)
{
    for (size_t i = length; i > 1; i--)
    {
        size_t j = (size_t)rand() % i;
        unsigned int tmp = vertices[i - 1];
        vertices[i - 1] = vertices[j];
        vertices[j] = tmp;
    }
}

/** @brief Index of val in vertices. val is guaranteed to be present. */
static size_t position(const unsigned int *vertices, size_t length, unsigned int val)
{
    size_t idx = 0;
    while (idx < length && vertices[idx] != val)
    {
        ++idx;
    }
    return idx;
}

/**
 * @brief Random vertex order -> keep every edge that points "backwards".
 *        The caller must free(result.NumberOfEdges).
 */
static struct EdgeList generateSolution(const struct EdgeList *graph)
{
    struct EdgeList rv = {0};
    size_t n = (size_t)graph->length;

    unsigned int *vertices = malloc(2 * n * sizeof *vertices);
    struct Edge *kept = malloc(n * sizeof *kept);
    if (vertices == NULL || kept == NULL)
    {
        error_exit("malloc failed");
    }

    size_t count = 0;
    for (size_t i = 0; i < n; i++)
    {
        addVertex(vertices, &count, graph->NumberOfEdges[i].u.node_number);
        addVertex(vertices, &count, graph->NumberOfEdges[i].v.node_number);
    }

    shuffleVertices(vertices, count);

    size_t keptCount = 0;
    for (size_t i = 0; i < n; i++)
    {
        size_t pu = position(vertices, count, graph->NumberOfEdges[i].u.node_number);
        size_t pv = position(vertices, count, graph->NumberOfEdges[i].v.node_number);

        if (pu > pv)
        {
            kept[keptCount++] = graph->NumberOfEdges[i];
        }
    }

    free(vertices);

    rv.NumberOfEdges = kept;
    rv.length = (ssize_t)keptCount;
    return rv;
}

/**
 * @brief Copies the solution into the next free slot of the circular buffer.
 *        Solutions with more than MAX_SOLUTION_EDGES edges are discarded.
 */
static void writeSolution(const struct EdgeList *solution, struct shm *sharedMemory)
{
    if ((size_t)solution->length > MAX_SOLUTION_EDGES)
    {
        return;
    }

    sem_wait(&sharedMemory->free);  /* wait for a free slot */
    sem_wait(&sharedMemory->write); /* exclusive access among generators */

    size_t slot = sharedMemory->writehead;
    sharedMemory->data[slot].count = (size_t)solution->length;
    for (size_t i = 0; i < (size_t)solution->length; i++)
    {
        sharedMemory->data[slot].edges[i] = solution->NumberOfEdges[i];
    }
    sharedMemory->writehead = (slot + 1) % MAX_BUFF_SIZE;

    sem_post(&sharedMemory->write);
    sem_post(&sharedMemory->used);

    fprintf(stdout, "Wrote %zu Edges at slot %zu\n", (size_t)solution->length, slot);
}

int main(int argc, char **argv)
{
    /* parse first so a bad command line never leaves shm/semaphores behind */
    struct EdgeList graph = parseInput(argc, argv);

    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    struct EdgeList solution = generateSolution(&graph);

    int shmfd;
    struct shm *possibleSolution = sharedMemory_Client(&shmfd);
    possibleSolution->free = initializeSemaphore_Client(FREE_SPACE_SEMAPHORE);
    possibleSolution->used = initializeSemaphore_Client(USED_SPACE_SEMAPHORE);
    possibleSolution->write = initializeSemaphore_Client(WRITE_SPACE_SEMAPHORE);

    writeSolution(&solution, possibleSolution);

    cleanSemaphore_Client(&possibleSolution->free);
    cleanSemaphore_Client(&possibleSolution->used);
    cleanSemaphore_Client(&possibleSolution->write);
    cleanSharedMemory_Client(possibleSolution, shmfd);

    free(solution.NumberOfEdges);
    free(graph.NumberOfEdges);
    return 0;
}
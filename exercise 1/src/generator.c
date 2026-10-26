/**
 * @file generator.c
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief
 * @version 0.1
 * @date 2026-10-20
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "../include/__semaphore_interface.h"

static inline struct EdgeList parseInput(int argc, char **argv)
{
    struct EdgeList rv = {0};

    /* Error checking*/
    if ((argc - 1) < 1)
    {
        error_exit("too few arguments");
    }

    if ((argc - 1) > 256)
    {
        error_exit("too many arguments");
    }

    for (int i = 1; i < argc; i++)
    {
        for (int j = i + 1; j < argc; j++)
        {
            if (strcmp(argv[i], argv[j]) == 0)
            {
                error_exit("duplicate arguments");
            }
        }
    }

    for (int i = 1; i < argc; i++)
    {
        if (strchr(argv[i], '-') == NULL)
        {
            error_exit("no dash in argument");
        }
    }

    for (int i = 1; i < argc; i++)
    {
        if (strchr(argv[i], '-') != strrchr(argv[i], '-'))
        {
            error_exit("to many dashes in argument");
        }
    }

    for (int i = 1; i < argc; i++)
    {
        char *string = strdup(argv[i]);
        assert(string != NULL);

        char *savePtr = NULL;
        char *first = strtok_r(string, "-", &savePtr);
        char *second = strtok_r(NULL, "-", &savePtr);

        if ((first == NULL) || (second == NULL))
        {
            error_exit("missing nodes in arguments");
        }

        for (size_t i = 0; i < strlen(first); i++)
        {
            if (!isdigit(first[i]))
            {
                error_exit("No digits in argument node");
            }
        }

        for (size_t i = 0; i < strlen(second); i++)
        {
            if (!isdigit(second[i]))
            {
                error_exit("No digits in argument node");
            }
        }

        free(string);
    }

    for (int i = 1; i < argc; ++i)
    {
        char *string = strdup(argv[i]);
        assert(string != NULL);
        char *savePtr = NULL;
        char *first = strtok_r(string, "-", &savePtr);
        char *second = strtok_r(NULL, "-", &savePtr);

        rv.NumberOfEdges[i - 1].u.node_number = strtol(first, NULL, 10);
        rv.NumberOfEdges[i - 1].v.node_number = strtol(second, NULL, 10);
        free(string);
    }
    rv.length = (ssize_t)(argc - 1);
    return rv;
}

static inline void shuffleVertexes(struct Node NodeList[], size_t length)
{
    if (length > 1)
    {
        for (size_t i = 0; i < length; ++i)
        {
            size_t j = i + rand() / (RAND_MAX / (length - i) + 1);
            struct Node tmp = NodeList[j];
            NodeList[j] = NodeList[i];
            NodeList[i] = tmp;
        }
    }
}

static size_t position(struct Node *NodeList, uint valueNode, size_t length)
{
    size_t position = 0;

    while (position < length && NodeList[position].node_number != valueNode)
    {
        ++position;
    }

    return (position == length ? -1 : position);
}

static inline struct EdgeList const generateSolution(const struct EdgeList *restrict graph)
{
    struct EdgeList rv = {0};
    struct Node NodeList[graph->length * 2];

    int counter = 0, pos = 0;

    for (size_t i = 0; i < graph->length; ++i)
    {
        struct Node edge[2] = {graph->NumberOfEdges->u, graph->NumberOfEdges->v};

        for (size_t j = 0; j < 2; ++j)
        {
            NodeList[pos++] = edge[j];
        }
    }

    shuffleVertexes(NodeList, graph->length * 2);

    for (int i = 0; i < graph->length; i++)
    {
        size_t pu = position(NodeList, graph->NumberOfEdges[i].u.node_number, graph->length * 2);
        size_t pv = position(NodeList, graph->NumberOfEdges[i].v.node_number, graph->length * 2);

        if (pu > pv)
        {
            rv.NumberOfEdges[counter++] = graph->NumberOfEdges[i];
        }
    }

    rv.length = counter;

    return rv;
}

void writeSolution(const struct EdgeList graph, struct shm *sharedMemory, sem_t free, sem_t used, sem_t write)
{
    sem_wait(&write);
    sem_wait(&free);
    sharedMemory->data[sharedMemory->writehead] = graph;
    sem_post(&write);
    sem_post(&used);
    sharedMemory->writehead = (sharedMemory->writehead + 1) % MAX_BUFF_SIZE;
    fprintf(stdout, "Wrote %zu Edges at slot %zu\n", graph.length, sharedMemory->writehead);
}

int main(int argc, char **argv)
{
    int shmfd;
    struct shm *possibleSolution = sharedMemory_Client(&shmfd);
    sem_t *free, *used, *write;

    /*What the fuck, why the fuck would I fucking ever need the fucking pid randomization fucking claude fucking cunt
     * fucking fuck average LLM clanker retardation, useless garbage piece of fucking shit, deranger fucking retard,
     * will delete later, felt cute*/
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());

    free = initializeSemaphore_Client(FREE_SPACE_SEMAPHORE);
    used = initializeSemaphore_Client(USED_SPACE_SEMAPHORE);
    write = initializeSemaphore_Client(WRITE_SPACE_SEMAPHORE);

    struct EdgeList test = parseInput(argc, argv);
    writeSolution(generateSolution(&test), possibleSolution, *free, *used, *write);

    cleanSemaphore_Client(free);
    cleanSemaphore_Client(used);
    cleanSemaphore_Client(write);
    cleanSharedMemory_Client(possibleSolution, shmfd);
    return 0;
}
/**
 * @file generator.c
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief This is the generator program. The code is formatted in knfmt to follow the rules of
 * knf(https://man.openbsd.org/style)
 * @version 0.1
 * @date 2026-10-20
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "__semaphore_interface.h"

/**
 * @brief prints a usage prompt, can be called if an error occurs, unlike the supervisor, the generator doesn't have an
 * explicit CLI handling
 *
 */
static inline void usage(void)
{
    printf("Usage: ./generator Edges...\n");
    printf("An edge consists of a number u a dash and a node v. For example 0-1 is a valid edge, after listing an "
           "another edge, a space is needed\n");
    return;
}

/**
 * @brief stores the edges into an EdgeList, for the structure of the graph see include/graph.h
 *
 * @param argc
 * @param argv
 * @return struct EdgeList
 */
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

/**
 * @brief needed for the algorithm, see assets/algorithm.md
 *
 * @param NodeList
 * @param length
 */
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

/**
 * @brief needed for the algorithm, see assets/algorithm.md
 *
 * @param NodeList the given nodes of a graph, optionally shuffeld
 * @param valueNode the number assigned to a node, preferrably the one given EdgeList
 * @param length every EdgeList has also a length, put it here to determine the position of specific node
 * @return size_t
 */
static size_t position(struct Node *NodeList, uint valueNode, size_t length)
{
    size_t position = 0;

    while (position < length && NodeList[position].node_number != valueNode)
    {
        ++position;
    }

    return (position == length ? -1 : position);
}

/**
 * @brief Guesses a possible solution, using a monte carlo approach, for more see assets/algorithm.md
 *
 * @param graph
 * @return struct EdgeList const
 */
static inline struct EdgeList const generateSolution(const struct EdgeList *restrict graph)
{
    struct EdgeList rv = {0};
    struct Node NodeList[graph->length * 2];

    int counter = 0, pos = 0;

    for (size_t i = 0; i < graph->length; ++i)
    {
        struct Node edge[2] = {graph->NumberOfEdges[i].u, graph->NumberOfEdges[i].v};

        for (size_t j = 0; j < 2; ++j)
        {
            /* Writes the nodes only once into the NodeList make it more uniqe */
            if (position(NodeList, edge[j].node_number, counter) == -1)
            {
                NodeList[pos++] = edge[j];
            }
        }
    }

    shuffleVertexes(NodeList, pos);

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

    // printf("rv length = %zd\n", rv.length);

    return rv;
}

/**
 * @brief writes the solution into the shared memory in /dev/shm. IPC guaranteed by the semaphores free, used and write
 *
 * @param graph the graph, which is hopefully a possible solution
 * @param sharedMemory the shared memory in /dev/shm
 * @param free IPC - free semaphore
 * @param used IPC - used semaphore
 * @param write IPC - write semaphore
 */
void writeSolution(const struct EdgeList graph, struct shm *sharedMemory, sem_t *free, sem_t *used, sem_t *write)
{
    if (sharedMemory->status == 1)
    {
        if (sem_wait(free) == -1)
        {
            error_exit("sem_wait (free)");
        }
        if (sharedMemory->status != 1)
        {
            sem_post(free);
            return;
        }
        if (sem_wait(write) == -1)
        {
            error_exit("sem_wait (write)");
        }

        /*Write only valid graphs*/
        if (graph.length < (MAX_SOLUTION_EDGES + 1))
        {
            sharedMemory->data[sharedMemory->writehead] = graph;
            sharedMemory->writehead = (sharedMemory->writehead + 1) % MAX_BUFF_SIZE;
        }

        if (sem_post(write) == -1)
        {
            error_exit("sem_post (write)");
        }
        if (sem_post(used) == -1)
        {
            error_exit("sem_post (used)");
        }
        fprintf(stderr, "Wrote %zu Edges at slot %zu\n", graph.length, sharedMemory->writehead);
    }
}

/**
 * @brief main, opens the shared memory and the semaphores. Parses a graph from the input, writes a possible
 * solution,generated from input, into the sahred memory.
 *
 * @param argc argument counter
 * @param argv argument vector(s)
 * @return int 0 on success else -1, see error cases on shared memory and semaphores on
 * include/__semaphore_interface.h
 */
int main(int argc, char **argv)
{

    // AUCH WENN MAN KEINE ARGUMENTE EINSEZTEN MUSS ARGUMENT PARSING IENBAUEN

    int shmfd;
    struct shm *possibleSolution = sharedMemory_Client(&shmfd);
    sem_t *freeSem, *usedSem, *writeSem;

    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());

    freeSem = initializeSemaphore_Client(FREE_SPACE_SEMAPHORE);
    usedSem = initializeSemaphore_Client(USED_SPACE_SEMAPHORE);
    writeSem = initializeSemaphore_Client(WRITE_SPACE_SEMAPHORE);

    struct EdgeList test = parseInput(argc, argv);
    while (possibleSolution->status == 1)
    {
        writeSolution(generateSolution(&test), possibleSolution, freeSem, usedSem, writeSem);
    }

    cleanSemaphore_Client(freeSem);
    cleanSemaphore_Client(usedSem);
    cleanSemaphore_Client(writeSem);
    cleanSharedMemory_Client(possibleSolution, shmfd);
    return 0;
}

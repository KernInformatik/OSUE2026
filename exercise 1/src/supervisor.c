/**
 * @file supervisor.c
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief
 * @version 0.1
 * @date 2026-10-25
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "../include/__semaphore_interface.h"

extern volatile sig_atomic_t quit;

/**
 * @brief is the usage function, to print the synopsis of supervisor
 *
 */
static inline void
usage(FILE *Output)
{
	fprintf(Output,
	    "Usage: ./supervisor [-h] [-o outputfile] [-n limit] [-w delay]\n");
	return;
}

/**
 * @brief delays time, given from the input
 *
 * @param seconds
 */
static void
delay(uint seconds)
{
	sleep(seconds);
}

/**
 * @brief is the current solution, needed to check the validity of a solution
 *
 */
static long currenBest = SSIZE_MAX;

/**
 * @brief reads a possible solution from the shm (circular buffer)
 *
 * @param buffer  the shm object, needs to be created see assertion
 * @param Output specifies the output file
 * @return int 1 for a cyclic graph 0 for an acyclic graph
 */
static int
readSolution(struct shm *buffer, FILE *Output)
{
	assert(buffer != NULL);
	struct EdgeList solution = buffer->data[buffer->readhead];
	buffer->readhead = (buffer->readhead + 1) % MAX_BUFF_SIZE;

	if (solution.length == 0) {
		return 1;
	}

	if (solution.length < currenBest) {
		currenBest = solution.length;

		fprintf(stderr, "Solution with %zd edges:", solution.length);
		for (size_t i = 0; i < solution.length; ++i) {
			fprintf(stderr, " %d-%d",
			    solution.NumberOfEdges[i].u.node_number,
			    solution.NumberOfEdges[i].v.node_number);
		}
		fprintf(stderr, "\n");

		if (Output != NULL) {
			fprintf(Output, "Solution with %zd edges:",
			    solution.length);
			for (size_t i = 0; i < solution.length; ++i) {
				fprintf(Output, " %d-%d",
				    solution.NumberOfEdges[i].u.node_number,
				    solution.NumberOfEdges[i].v.node_number);
			}
			fprintf(Output, "\n");
			fflush(Output);
		}
	}
	return 0;
}

/**
 * @brief parses the given input, checks it and returns it as an unsigned integer
 *
 * @param s optionally the optarg string
 * @return uint
 */
static uint
parseInput(const char *s)
{
	long long rv = 0;
	assert(s != NULL);
	char *tmp = strdup(s);
	char *end = NULL;
	rv = strtoll(tmp, &end, 10);
	if (tmp == end || *end != '\0' || rv < 0 || rv > LONG_MAX) {
		free(tmp);
		usage(stderr);
		error_exit("Invalid number, or different error occured");
	}
	free(tmp);
	return (uint)rv;
}

/**
 * @brief main function
 *
 * @param argc the argument counter
 * @param argv argument vector
 * @return int 0 on success
 */
int
main(int argc, char **argv)
{
	int opt, seconds = 0, limit = 0;
	bool setDelay = false, setLimit = false;

	FILE *out = NULL;

	while ((opt = getopt(argc, argv, "ho:n:w:")) != -1) {
		switch (opt) {
		case 'h':
			usage(stdout);
			exit(EXIT_SUCCESS);
			break;
		case 'o':
			out = fopen(optarg, "w+");
			if (out == NULL) {
				error_exit("fopen");
			}
			break;
		case 'n':
			setLimit = true;
			limit = parseInput(optarg);
			break;
		case 'w':
			setDelay = true;
			seconds = parseInput(optarg);
			break;
		default:
			usage(stderr);
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (optind < argc) {
		printBuddha();
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	int shmfd;
	struct shm *sharedMemory = sharedMemory_Server(&shmfd);
	sem_t *freeSem, *usedSem, *writeSem;
	freeSem = initializeSemaphore_Server(FREE_SPACE_SEMAPHORE,
	    FREE_SPACE_SEMAPHORE_SIZE);
	writeSem = initializeSemaphore_Server(WRITE_SPACE_SEMAPHORE,
	    WRITE_SPACE_SEMAPHORE_SIZE);
	usedSem = initializeSemaphore_Server(USED_SPACE_SEMAPHORE,
	    USED_SPACE_SEMAPHORE_SIZE);
	initSignalHandler();
	sharedMemory->status = 1;

    /*End of the CLI */

    /**
     * @brief The beginning of pure menace
     *
     */
	if (setDelay && seconds > 0) {
		delay(seconds);
	}

	int count = 0, stop = 0;

	while (!quit && (!setLimit || count < limit)) {
		if (sem_wait(usedSem) == -1) {
			if (errno == EINTR) {
				continue;
			}
			error_exit("Couldn't initialize sempahore");
		}
		if (quit) {
			sharedMemory->status = 0;
			break;
		}
		stop = readSolution(sharedMemory, out);
		count++;

		if (sem_post(freeSem) == -1) {
			printBuddha();
			error_exit("sem_post (free)");
		}
		if (stop) {
			break;
		}
	}

	sharedMemory->status = 0;
	if (stop) {
		printf("The graph is acyclic!\n");
		if (out != NULL)
			fprintf(out, "The graph is acyclic!\n");
	} else if (currenBest != SSIZE_MAX) {
		printf("The graph might not be acyclic, best solution removes %zd edges.\n",
		    currenBest);
		if (out != NULL)
			fprintf(out,
			    "The graph might not be acyclic, best solution removes %zd edges.\n",
			    currenBest);
	}

    /*Not quite necessecary, but good behaviour*/
    /*see https://jprofesorek.mooo.com/jkonczak/os_cp:semaphores */
	for (int i = 0; i < MAX_BUFF_SIZE; ++i) {
		sem_post(freeSem);
	}
	cleanSemaphore_Server(freeSem, FREE_SPACE_SEMAPHORE);
	cleanSemaphore_Server(usedSem, USED_SPACE_SEMAPHORE);
	cleanSemaphore_Server(writeSem, WRITE_SPACE_SEMAPHORE);

	cleanSharedMemory_Server(sharedMemory, shmfd);

	if (out != NULL) {
		fclose(out);
	}

	return EXIT_SUCCESS;
}

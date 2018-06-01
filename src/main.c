#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <pthread.h>

#include "tests.h"
#include "thread.h"

/* 1 to 128 threads */
#define MAX_THREADS (1 << 7)
#define MIN_THREADS (1 << 0)

/* 16 to 67108864 bytes per block */
#define MAX_SIZE (1 << 26)
#define MIN_SIZE (1 << 4)

/* 1 to 1048576 blocks */
#define MAX_COUNT (1 << 20)
#define MIN_COUNT (1 << 0)

int main(int argc, char * argv[])
{
	printf("iotest 0.02\nWritten by Robin Andersson. (c13ras)\n");

	/* Option variables for safe keeping of default parameters. */

	uint32_t opt_thread_count			= MIN_THREADS;

	void *(*opt_test_function)(void *)	= test_seq;

	uint32_t opt_block_size				= MIN_SIZE;
	uint32_t opt_block_count			= MIN_COUNT;

	char * opt_path						= NULL;

	/* Scan the argument list for any options and filter them out
	 * with getopt(), then grab the input path if it exists. */

	int32_t opt;
	while((opt = getopt(argc, argv, "n:t:b:B:")) != -1)
	{
		switch(opt)
		{
			case 'n':
				/* Number of threads. */
				opt_thread_count = atoi(optarg);

				opt_thread_count = (opt_thread_count > MAX_THREADS) ? MAX_THREADS : opt_thread_count;
				opt_thread_count = (opt_thread_count < MIN_THREADS) ? MIN_THREADS : opt_thread_count;

				break;

			case 't':
				/* Test method. */
				if(!strcmp("seq", optarg))
				{
					opt_test_function = test_seq;
				}
				else if(!strcmp("rand", optarg))
				{
					srand(time(NULL));
					opt_test_function = test_rand;
				}
				else
				{
					fprintf(stderr, "invalid test\n");
					return EXIT_FAILURE;
				}

				break;

			case 'b':
				/* Block size. */
				opt_block_size = atoi(optarg);

				opt_block_size = (opt_block_size > MAX_SIZE) ? MAX_SIZE : opt_block_size;
				opt_block_size = (opt_block_size < MIN_SIZE) ? MIN_SIZE : opt_block_size;

				break;

			case 'B':
				/* Block count. */
				opt_block_count = atoi(optarg);

				opt_block_count = (opt_block_count > MAX_COUNT) ? MAX_COUNT : opt_block_count;
				opt_block_count = (opt_block_count < MIN_COUNT) ? MIN_COUNT : opt_block_count;

				break;

			default:
				printf("\nUSAGE: iotest [-n threads] [-t seq/rand] [-b block size] [-B block count] input\n\n");
				return EXIT_SUCCESS;
		}
	}

	/* Copy the path to heap memory if it exists.
	 * Otherwise, complain like there's no tomorrow. */

	if(optind >= argc)
	{
		fprintf(stderr, "missing test input file path\n");
		return EXIT_FAILURE;
	}

	uint32_t opt_path_len = strlen(argv[optind]);

	opt_path = malloc(opt_path_len * sizeof(char));
	if(!opt_path)
	{
		fprintf(stderr, "path buffer allocation failure\n");
		return EXIT_FAILURE;
	}

	strncpy(opt_path, argv[optind], opt_path_len);

	/* Allocate memory for the thread data structures. */

	struct thread_info * threads = malloc(opt_thread_count * sizeof(struct thread_info));
	if(!threads)
	{
		fprintf(stderr, "thread allocation failure\n");

		free(opt_path);
		return EXIT_FAILURE;
	}

	/* Prepare thread parameters and create the threads. */

	for(int i = 0; i < opt_thread_count; i++)
	{
		threads[i].path = opt_path;

		threads[i].block_size = opt_block_size;
		threads[i].block_count = opt_block_count;

		pthread_create(&threads[i].thread, NULL, opt_test_function, (void *)&threads[i]);
	}

	/* Now, join the threads in the same order.
	 * This wont affect the test results since the results are generated in
	 * the threads themselves, we just display them in a proper manner. */

	for(int i = 0; i < opt_thread_count; i++)
	{
		pthread_join(threads[i].thread, NULL);

		printf("%d %f\n", i, threads[i].t_total);
	}

	/* Clean up after finishing everything. */

	free(opt_path);
	free(threads);

	return EXIT_SUCCESS;
}

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define NUM_THREADS 5

/* Test information structure. */
struct worker_args
{
	char * path;
	int len;
};

/*
 * worker_seqread()
 * Sequential read worker function, meant to put heavy load on I/O schedulers
 * that expect quick and easy I/O requests, like the Noop scheduler.
 */
void * worker_seqread(void * input)
{
	struct worker_args * args = input;

	char * dataBuf = malloc(args->len);
	if(!dataBuf)
	{
		/* Somehow we didn't get enough memory for the buffer, abort! */
		printf("Error: Unable to allocate memory buffer, aborting!\n");
		return NULL;
	}

	/* Now open the file in read-only mode. If it breaks, PRAY FOR MOJO.
	 * Unfortunately, this does not work for some reason and I have yet
	 * to figure out why. Late-night coding, not even once. */

	int fd = open(args->path, O_RDONLY | O_DIRECT);
	if(fd < 0)
	{
		printf("Error: Unable to open %s in read-only mode, aborting!\n", args->path);
		return NULL;
	}

	/* Start measuring the time with a timer. */

	long bytesTotal = 0;

	struct timespec ts, te;
	clock_gettime(CLOCK_REALTIME, &ts);

	/* Now, start reading from start to finish, abort if something comes up. */

	int rdBytes;
	while((rdBytes = read(fd, dataBuf, args->len)) != 0)
	{
		bytesTotal += rdBytes;
	}

	close(fd);
	free(dataBuf);

	/* We're done, wrap it up and calculate how long it took. */

	clock_gettime(CLOCK_REALTIME, &te);
	long tTotal = 0;

	/* Adjust the time accordingly because POSIX timers are a PITA
	   and will loop the nanosecond counter every second. */
	if((te.tv_nsec - ts.tv_nsec) < 0)
	{
		tTotal = 1E9 + (te.tv_nsec - ts.tv_nsec);
	}
	else
	{
		tTotal = te.tv_nsec - ts.tv_nsec;
	}

	tTotal += ((te.tv_sec - ts.tv_sec) * 1000);
	tTotal /= 1E6;

	printf("Finished execution in %d milliseconds.\n", tTotal);
	return NULL;
}

/*
 * worker_rndread()
 * Supposed to be practically the same as worker_seqread() but with
 * random-access reading instead to really shake up I/O schedulers
 * that don't really expect it, like the CFQ scheduler for example.
 */
void * worker_rndread()
{
	return NULL;
}

int main(int argc, char * argv[])
{
	printf("iotest 0.01\nWritten by Robin Andersson. (c13ras)\n");

	/* Only start if we have exactly 1 argument; the test file path. */

	if(argc != 2)
	{
		printf("USAGE: iotest <input file>\n\n");
		return 0;
	}

	/* Launch the sequential tests, pretty ugly way of doing it though. */

	pthread_t tThreads[NUM_THREADS];
	int testLen = 4096;

	for(int i = 0; i < 5; i++)
	{
		printf("Beginning %d byte sequential read test: \n\n", testLen);

		for(int t = 0; t < NUM_THREADS; t++)
		{
			/* Start the threads with the current buffer size. */
			struct worker_args args;

			args.path = argv[1];
			args.len = testLen;

			pthread_create(&tThreads[t], NULL, &worker_seqread, &args);
		}

		for(int t = 0; t < NUM_THREADS; t++)
		{
			/* Join the threads as soon as possible. */

			pthread_join(tThreads[t], NULL);
		}

		testLen <<= 3;
	}

	/* Begin random-access tests now that we're done with sequential tests. */

	return 0;
}

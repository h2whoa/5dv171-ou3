#define _GNU_SOURCE

#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

#include "tests.h"
#include "thread.h"

/*
 * timespec_diff
 * Utility function for calculating time differences in seconds.
 */
double timespec_diff(struct timespec * start, struct timespec * stop)
{
	double diff_s = stop->tv_sec - start->tv_sec;
	double diff_ns = stop->tv_nsec - start->tv_nsec;

	/* Calculating the second difference is easy (as seen above) but
	 * the nanosecond difference takes a little extra work since that
	 * field specifies nanoseconds since the previous second. */

	if(diff_ns < 0)
	{
		diff_ns += 1E9;
	}

	/* Blend the second and nanosecond fields together and return. */

	diff_s += (double)(diff_ns / 1E9);

	return diff_s;
}

/*
 * test_seq
 * Sequential read test, attempts to read X blocks of Y bytes in size.
 */
void * test_seq(void * args)
{
	/* Prepare per-thread variables and such before doing anything. */

	struct thread_info * info = (struct thread_info *)args;
	struct timespec t_total_start, t_total_stop;

	/* Attempt to allocate the read buffer first. */

	uint8_t * buffer = malloc(info->block_size * sizeof(uint8_t));
	if(!buffer)
	{
		fprintf(stderr, "buffer allocation failure\n");
		return NULL;
	}

	/* Now try to open the file, bail out if anything goes wrong. */

	int fd = open(info->path, O_RDONLY);
	if(fd < 0)
	{
		fprintf(stderr, "file access error\n");

		free(buffer);
		return NULL;
	}

	/* Begin the read test, measure the total time it takes as well. */

	clock_gettime(CLOCK_REALTIME, &t_total_start);
	for(int i = 0; i < info->block_count; i++)
	{
		int32_t status = read(fd, buffer, info->block_size);

		/* Abort if something goes wrong or end of file is reached. */

		if(status <= 0)
		{
			break;
		}
	}

	clock_gettime(CLOCK_REALTIME, &t_total_stop);
	info->t_total = timespec_diff(&t_total_start, &t_total_stop);

	/* Close the file and free the memory buffer before leaving. */

	close(fd);
	free(buffer);

	return NULL;
}

/*
 * test_rand
 * Random-access read test, attemps to read X blocks of Y bytes from 
 * random offsets in the test input. Doesn't care if the amount of
 * data read was less than the block size, hence the total number of
 * bytes may be somewhat lower than (block size * block count) bytes.
 */
void * test_rand(void * args)
{
	/* Prepare per-thread variables and such before doing anything. */

	struct thread_info * info = (struct thread_info *)args;
	struct timespec t_total_start, t_total_stop;

	uint8_t * buffer = malloc(info->block_size * sizeof(uint8_t));
	if(!buffer)
	{
		fprintf(stderr, "buffer allocation failure\n");
		return NULL;
	}

	/* Attempt to open the input file path, bail out if it doesn't work. */

	int fd = open(info->path, O_RDONLY);
	if(fd < 0)
	{
		fprintf(stderr, "file access error\n");

		free(buffer);
		return NULL;
	}

	off_t fd_end = lseek(fd, 0, SEEK_END);

	/* Begin the read test, measure the total time it takes as well. */

	clock_gettime(CLOCK_REALTIME, &t_total_start);

	for(int i = 0; i < info->block_count; i++)
	{
		int32_t status = read(fd, buffer, info->block_size);
		int32_t next_offset = rand() % fd_end;

		/* Don't freak out if we hit the end of file by accident. */
		
		if(status < 0)
		{
			break;
		}

		/* Jump to the next random offset. */

		lseek(fd, next_offset, SEEK_SET);
	}

	clock_gettime(CLOCK_REALTIME, &t_total_stop);
	info->t_total = timespec_diff(&t_total_start, &t_total_stop);

	/* Close the file and free the memory buffer before leaving. */

	close(fd);
	free(buffer);

	return NULL;
}
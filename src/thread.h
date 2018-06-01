#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <pthread.h>

/*
 * thread_info
 * Thread container structure, keeps track of data common to all threads.
 */
struct thread_info
{
	pthread_t thread;

	char * path;
	uint32_t block_size, block_count;

	double t_total;
};

#endif
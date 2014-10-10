
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "pqlist.h"
#include "pqheap.h"

size_t size = 0;

enum {
	TEST_LIST_PUSH = 0,
	TEST_HEAP_PUSH,
	TEST_LIST_POP,
	TEST_HEAP_POP,
	TEST_LIST_UPDATE,
	TEST_HEAP_UPDATE,
	TEST_LIST_ALL,
	TEST_HEAP_ALL,
	TEST_MAX,
};

const char *names[TEST_MAX] = {
		[TEST_LIST_PUSH]   = "List Push  ",
		[TEST_LIST_POP]    = "List Pop   ",
		[TEST_LIST_UPDATE] = "List Update",
		[TEST_LIST_ALL]    = "List All",
		[TEST_HEAP_PUSH]   = "Heap Push  ",
		[TEST_HEAP_POP]    = "Heap Pop   ",
		[TEST_HEAP_UPDATE] = "Heap Update",
		[TEST_HEAP_ALL]    = "Heap All",
};

struct timeval times[TEST_MAX][2];

#define start_measure(test) gettimeofday(&times[test][0], NULL);
#define stop_measure(test)  gettimeofday(&times[test][1], NULL);

void run_pqlist()
{
	struct pqlist pq;
	struct pqlist_entry es[size];
	struct pqlist_entry *e;

	pqlist_init(&pq);
	srand(0);
	int i;

	start_measure(TEST_LIST_ALL);

	start_measure(TEST_LIST_PUSH);
	for (i=0; i< size; i++) {
		pqlist_push(&pq, &es[i], rand());
	}
	stop_measure(TEST_LIST_PUSH);

	start_measure(TEST_LIST_UPDATE);
	for (i=0; i< size; i++) {
		pqlist_remove(&es[i]);
		pqlist_push(&pq, &es[i], rand());
	}
	stop_measure(TEST_LIST_UPDATE);

	start_measure(TEST_LIST_POP);
	for (i=0; i< size; i++) {
		e = pqlist_pop(&pq);
	}
	stop_measure(TEST_LIST_POP);

	stop_measure(TEST_LIST_ALL);
}

void run_pqheap()
{
	struct pqheap pq;
	struct pqheap_entry es[size];
	struct pqheap_entry *e;

	pqheap_init(&pq);
	srand(0);
	int i;

	start_measure(TEST_HEAP_ALL);

	start_measure(TEST_HEAP_PUSH);
	for (i=0; i< size; i++) {
		pqheap_push(&pq, &es[i], rand());
	}
	stop_measure(TEST_HEAP_PUSH);

	start_measure(TEST_HEAP_UPDATE);
	for (i=0; i< size; i++) {
		pqheap_remove(&pq, &es[i]);
		pqheap_push(&pq, &es[i], rand());
	}
	stop_measure(TEST_HEAP_UPDATE);

	start_measure(TEST_HEAP_POP);
	for (i=0; i< size; i++) {
		e = pqheap_pop(&pq);
	}
	stop_measure(TEST_HEAP_POP);

	stop_measure(TEST_HEAP_ALL);
}

void display_header()
{
	int i;
	printf("size\t");
	for(i = 0; i<TEST_MAX; i++) {
		printf("%s\t", names[i]);
	}
	printf("\n");
}

void display_measures()
{
	int i;
	printf("%d\t",(int) size);
	for(i = 0; i<TEST_MAX; i++) {
		printf("%f\t", times[i][1].tv_sec - times[i][0].tv_sec + ((double)(times[i][1].tv_usec - times[i][0].tv_usec))*(1.0/1000000));
	}
	printf("\n");
}

void main() {
	display_header();
	for(size = 1; size < (1<<16); size<<=1) {
		run_pqlist();
		run_pqheap();
		display_measures();
	}
}



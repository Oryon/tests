
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "btrie.h"
#include <arpa/inet.h>
#include <libubox/avl.h>

#define MAX_SIZE 1<<20
size_t size = 0;

enum {
	TEST_BTRIE_PUSH = 0,
	TEST_AVL_PUSH,
	TEST_BTRIE_ITERATE,
	TEST_AVL_ITERATE,
	TEST_BTRIE_LOOKUP,
	TEST_AVL_LOOKUP,
	TEST_BTRIE_REMOVE,
	TEST_AVL_REMOVE,
	TEST_BTRIE_ALL,
	TEST_AVL_ALL,
	TEST_MAX,
};

const char *names[TEST_MAX] = {
		[TEST_BTRIE_PUSH]    = "btrie Push   ",
		[TEST_BTRIE_ITERATE] = "btrie Iterate",
		[TEST_BTRIE_LOOKUP]  = "btrie Lookup ",
		[TEST_BTRIE_REMOVE]  = "btrie Remove ",
		[TEST_BTRIE_ALL]     = "btrie All    ",
		[TEST_AVL_PUSH]      = "avl Push     ",
		[TEST_AVL_ITERATE]   = "avl Iterate  ",
		[TEST_AVL_LOOKUP]    = "avl Lookup   ",
		[TEST_AVL_REMOVE]    = "avl Remove   ",
		[TEST_AVL_ALL]       = "avl All      ",
};

struct timeval times[TEST_MAX][2];

#define start_measure(test) gettimeofday(&times[test][0], NULL)
#define stop_measure(test)  gettimeofday(&times[test][1], NULL)

struct prefix {
	struct in6_addr prefix;
	uint8_t plen;
	struct avl_node ae;
	struct btrie_element be;

} p[MAX_SIZE];

void init_prefixes()
{
	int i;
	for (i=0; i< size; i++) {
		int j;
		for(j=0;j<16; j++) {
			p[i].prefix.s6_addr[j] = (uint8_t)rand();
		}
		p[i].plen = (uint8_t)rand();
		if(p[i].plen>128)
			p[i].plen -= 128;
	}
}

void run_btrie()
{
	struct btrie btrie;
	int i, k;
	struct prefix *prefix, *s;

	btrie_init(&btrie);

	start_measure(TEST_BTRIE_ALL);

	start_measure(TEST_BTRIE_PUSH);
	for (i=0; i< size; i++) {
		btrie_add(&btrie, &p[i].be, (btrie_key_t *)&p[i].prefix, p[i].plen);
	}
	stop_measure(TEST_BTRIE_PUSH);

	start_measure(TEST_BTRIE_ITERATE);
	btrie_for_each_down_entry(prefix, &btrie, NULL, 0, be) {
		k++;
	}
	stop_measure(TEST_BTRIE_ITERATE);

	start_measure(TEST_BTRIE_LOOKUP);
	for (i=0; i< size; i++) {
		btrie_for_each_entry(prefix, &btrie, (btrie_key_t *)&p[i].prefix, p[i].plen, be) {
			k++;
			break;
		}
	}
	stop_measure(TEST_BTRIE_LOOKUP);

	start_measure(TEST_BTRIE_REMOVE);
	for (i=0; i< size; i++) {
		btrie_remove(&p[i].be);
	}
	stop_measure(TEST_BTRIE_REMOVE);

	stop_measure(TEST_BTRIE_ALL);
}

static int bmemcmp(const void *m1, const void *m2, size_t bitlen)
{
	size_t bytes = bitlen >> 3;
	int r;
	if( (r = memcmp(m1, m2, bytes)) )
		return r;

	uint8_t rembit = bitlen & 0x07;
	if(!rembit)
		return 0;

	uint8_t *p1 = ((uint8_t *) m1) + bytes;
	uint8_t *p2 = ((uint8_t *) m2) + bytes;
	uint8_t mask = (0xff >> (8 - rembit)) << (8 - rembit);

	return ((int) (*p1 & mask)) - ((int) (*p2 & mask));
}


static int avl_comp(const void *k1, const void *k2, void *ptr)
{
	struct prefix *p1 = k1, *p2 = k2;
	uint8_t minlen = (p1->plen > p2->plen)?p2->plen:p1->plen;
	int i;
	if((i = bmemcmp(&p1->prefix, &p2->prefix, minlen)))
		return i;
	if(p1->plen > p2->plen)
		return -1;
	return 1;
}

void run_avl()
{
	struct avl_tree avl;
	struct avl_node *n;
	int i, k;
	struct prefix *entry, *s;

	avl_init(&avl, avl_comp ,true ,NULL);

	start_measure(TEST_AVL_ALL);

	start_measure(TEST_AVL_PUSH);
	for (i=0; i< size; i++) {
		p[i].ae.key = &p[i];
		avl_insert(&avl, &p[i].ae);
	}
	stop_measure(TEST_AVL_PUSH);

	start_measure(TEST_AVL_ITERATE);
	avl_for_each_element(&avl, entry, ae) {
		k++;
	}
	stop_measure(TEST_AVL_ITERATE);

	start_measure(TEST_AVL_LOOKUP);
	for (i=0; i< size; i++) {
		avl_find(&avl, &p[i]);
	}
	stop_measure(TEST_AVL_LOOKUP);

	start_measure(TEST_AVL_REMOVE);
	for (i=0; i< size; i++) {
		avl_delete(&avl, &p[i].ae);
	}
	stop_measure(TEST_AVL_REMOVE);

	stop_measure(TEST_AVL_ALL);
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
	init_prefixes();
	display_header();
	for(size = 1; size <= MAX_SIZE; size<<=1) {
		run_btrie();
		run_avl();
		display_measures();
	}
}



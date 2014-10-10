#include "pqheap.h"

#include <string.h>
#include <stdio.h>

#define BASE_SIZE 4

void pqheap_init(struct pqheap *pq)
{
	pq->count = 0;
	pq->size = 0;
	pq->heap = NULL;
	return;
}

static int pqheap_change_size(struct pqheap *pq, size_t size)
{
	struct pqheap_entry **heap;
	if(!(heap = realloc(pq->heap, size * sizeof(struct pqheap_entry *)))) {
		return -1;
	}
	pq->size = size;
	pq->heap = heap;
	return 0;
}

int pqheap_push(struct pqheap *pq, struct pqheap_entry *e, int priority)
{
	if(pq->size == pq->count && pqheap_change_size(pq, pq->size?(pq->size * 2):BASE_SIZE))
		return -1;

	e->priority = priority;

	struct pqheap_entry **heap = pq->heap;
	struct pqheap_entry *swap;
	size_t current = (pq->count++), parent;
	heap[current] = e;
#ifdef PQHEAP_KEEP_POS
	e->pos = current;
#endif
	while(current) {
		parent = (current - 1)/2;
		if(heap[current]->priority >= heap[parent]->priority)
			break;
		swap = heap[current];
		heap[current] = heap[parent];
		heap[parent] = swap;
#ifdef PQHEAP_KEEP_POS
		heap[current]->pos = current;
		heap[parent]->pos = parent;
#endif
		current = parent;
	}
	return 0;
}

static void pqheap_bubble_down(struct pqheap *pq, size_t current)
{
	struct pqheap_entry **heap = pq->heap, *swap;
	size_t son;
	while((son = current * 2 + 1) < pq->count) {
		if((son + 1) < pq->count && heap[son + 1]->priority < heap[son]->priority)
			son++;
		if(heap[current]->priority <= heap[son]->priority)
			break;
		swap = heap[current];
		heap[current] = heap[son];
		heap[son] = swap;
#ifdef PQHEAP_KEEP_POS
		heap[current]->pos = current;
		heap[son]->pos = son;
#endif
		current = son;
	}
}

static struct pqheap_entry *pqheap_del(struct pqheap *pq, size_t pos)
{
	struct pqheap_entry **heap = pq->heap;
	struct pqheap_entry *ret = heap[pos];
	heap[pos] = heap[--pq->count];
	pqheap_bubble_down(pq, pos);

	if(pq->count < pq->size / 4)
		pqheap_change_size(pq, pq->size / 2);
	return ret;
}

struct pqheap_entry *pqheap_pop(struct pqheap *pq)
{
	return pqheap_del(pq, 0);
}

void pqheap_remove(struct pqheap *pq, struct pqheap_entry *e)
{

#ifdef PQHEAP_KEEP_POS
	pqheap_del(pq, e->pos);
#else
	//Unfortunalty, we need to find it first. Linear time.
	struct pqheap_entry **cur = pq->heap;
	while(*cur != e)
		cur++;
	pqheap_del(pq, cur - pq->heap);
#endif


}

void pqheap_print(struct pqheap *pq)
{
	size_t i = 0;
	for(i = 0; i<pq->count; i++) {
		printf("%p: %d\n", pq->heap[i], pq->heap[i]->priority);
	}
}

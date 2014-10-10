
#ifndef PQHEAP_H_
#define PQHEAP_H_

#include <unistd.h>
#include <stdlib.h>

#define PQHEAP_KEEP_POS

struct pqheap_entry {
	int priority;
#ifdef PQHEAP_KEEP_POS
	size_t pos;
#endif
};

struct pqheap {
	size_t size;
	size_t count;
	struct pqheap_entry **heap;
};

void pqheap_init(struct pqheap *pq);
#define pqheap_is_empty(pq) ((pq)->count == 0)
void pqheap_remove(struct pqheap *pq, struct pqheap_entry *e);
struct pqheap_entry *pqheap_pop(struct pqheap *pq);
int pqheap_push(struct pqheap *pq, struct pqheap_entry *e, int priority);
void pqheap_print(struct pqheap *pq);


#endif /* PQHEAP_H_ */

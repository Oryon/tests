/*
 * pqlist.h
 *
 *  Created on: 4 oct. 2014
 *      Author: homenet
 */

#ifndef PQLIST_H_
#define PQLIST_H_

#include <libubox/list.h>

struct pqlist {
	struct list_head l;
};

struct pqlist_entry {
	struct list_head le;
	int priority;
};

#define pqlist_init(pq) INIT_LIST_HEAD(&(pq)->l)
#define pqlist_is_empty(pq) list_empty(&(pq)->l)
#define pqlist_remove(e) list_del(&(e)->le)
struct pqlist_entry *pqlist_pop(struct pqlist *pq);
void pqlist_push(struct pqlist *pq, struct pqlist_entry *e, int priority);


#endif /* PQLIST_H_ */

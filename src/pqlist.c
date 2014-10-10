/*
 * pqlist.c
 *
 *  Created on: 4 oct. 2014
 *      Author: homenet
 */

#include "pqlist.h"

struct pqlist_entry *pqlist_pop(struct pqlist *pq)
{
	struct list_head *h = pq->l.next;
	list_del(h);
	return container_of(h, struct pqlist_entry, le);
}

void pqlist_push(struct pqlist *pq, struct pqlist_entry *e, int priority)
{
	struct pqlist_entry *o;
	e->priority = priority;
	list_for_each_entry(o, &pq->l, le) {
		if(o->priority >= priority) {
			list_add_tail(&e->le, &o->le);
			return;
		}
	}
	list_add_tail(&e->le, &pq->l);
}

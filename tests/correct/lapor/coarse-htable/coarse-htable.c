#ifndef __COARSE_HTABLE_H__
#define __COARSE_HTABLE_H__

#include <stdbool.h>
#include "list.h"

/* API that the Driver must implement */
int get_thread_num();

struct htable_entry *new_node(int val);
void free_node(struct htable_entry *node);

/* Hashtable definition */
#define LOCK(l)   pthread_mutex_lock(&(l))
#define UNLOCK(l) pthread_mutex_unlock(&(l))

#define HTABLE_CAPACITY 16
#define HASH(x) (x) % HTABLE_CAPACITY

struct htable_entry {
	int val;
	struct list_head list;
};

struct htable_bucket {
	struct list_head list;
};

struct htable {
	struct htable_bucket table[HTABLE_CAPACITY];
	size_t size;
	pthread_mutex_t lock;
};

#define __HTABLE_INITIALIZER() { .lock = PTHREAD_MUTEX_INITIALIZER }
#define DEFINE_HTABLE(name)				\
	struct htable name = __HTABLE_INITIALIZER();

void htable_init(struct htable *ht)
{
	for (int i = 0; i < HTABLE_CAPACITY; i++)
		INIT_LIST_HEAD(&ht->table[i].list);
	/* ht->size = 0; */
}

void add(struct htable *ht, int val)
{
	int key = HASH(val);

	LOCK(ht->lock);
	struct htable_entry *entry = new_node(val);
	list_add_tail(&entry->list, &ht->table[key].list);
	/* ++ht->size; */
	UNLOCK(ht->lock);
}

bool contains(struct htable *ht, int val)
{
	int key = HASH(val);
	int retval = false;
	struct htable_entry *curr;

	LOCK(ht->lock);
	list_for_each_entry(curr, &ht->table[key].list, list) {
		if (curr->val == val) {
			retval = true;
			break;
		}
	}
	UNLOCK(ht->lock);
	return retval;
}

bool remove(struct htable *ht, int val)
{
	int key = HASH(val);
	int retval = false;
	struct htable_entry *curr;

	LOCK(ht->lock);
	list_for_each_entry(curr, &ht->table[key].list, list) {
		if (curr->val == val) {
			list_del(&curr->list);
			--ht->size;
			free_node(curr);
			retval = true;
			break;
		}
	}
	UNLOCK(ht->lock);
	return retval;
}

#endif /* __COARSE_HTABLE_H__ */

#ifndef __FINE_SET_H__
#define __FINE_SET_H__

#include <stdbool.h>
#include "list.h"

/* API that the Driver must implement */
int get_thread_num();

struct set_node *new_node(int key, int elem);
void free_node(struct set_node *node);

/* Set definition */
#define lock(l)   pthread_mutex_lock(&(l))
#define unlock(l) pthread_mutex_unlock(&(l))

#define hash(x) (x)

struct set_node {
	int key;
	int val;
	pthread_mutex_t lock;
	struct list_head list;
};

struct set_head {
	pthread_mutex_t lock;
	struct list_head list;
};

void set_init(struct set_head *set)
{
	INIT_LIST_HEAD(&set->list);
}

bool insert(struct set_node *curr, int key, int elem)
{
	if (key == curr->key)
		return false;

	struct set_node *node = new_node(key, elem);
	list_add_tail(&node->list, &curr->list);
	return true;
}

bool delete(struct set_node *curr, int key)
{
	if (key != curr->key)
		return false;

	list_del(&curr->list);
	return true;
}

bool contains(struct set_head *set, int elem)
{
 	int key = hash(elem);
	struct set_node *curr;
	struct list_head *pred; /* entry can be set_head or set_node */
	int retval = false;

	lock(set->lock);
	pred = &set->list;

	if (list_empty(&set->list))
		goto unlock_pred;

	list_for_each_entry(curr, &set->list, list) {
		lock(curr->lock);
		if (curr->key == key) {
			retval = true;
			goto unlock_curr;
		}
		if (curr->key > key)
			goto unlock_curr;

		if (pred == &set->list)
			unlock(container_of(pred, struct set_head, list)->lock);
		else
			unlock(container_of(pred, struct set_node, list)->lock);
		pred = &curr->list;
	}

	goto unlock_pred;

unlock_curr:
	unlock(curr->lock);
unlock_pred:
	if (pred == &set->list)
		unlock(container_of(pred, struct set_head, list)->lock);
	else
		unlock(container_of(pred, struct set_node, list)->lock);
	return retval;
}

bool add(struct set_head *set, int elem)
{
 	int key = hash(elem);
	struct set_node *curr;
	struct list_head *pred; /* entry can be set_head or set_node */
	int retval = true;

	lock(set->lock);
	pred = &set->list;

	if (list_empty(&set->list)) {
		struct set_node *node = new_node(key, elem);
		list_add(&node->list, &set->list);
		goto unlock_pred;
	}

	list_for_each_entry(curr, &set->list, list) {
		lock(curr->lock);
		if (curr->key >= key) {
			retval = insert(curr, key, elem);
			goto unlock_curr;
		}
		if (pred == &set->list)
			unlock(container_of(pred, struct set_head, list)->lock);
		else
			unlock(container_of(pred, struct set_node, list)->lock);
		pred = &curr->list;
	}

	lock(set->lock);
	struct set_node *node = new_node(key, elem);
	list_add_tail(&node->list, &set->list);
	unlock(set->lock);
	goto unlock_pred;

unlock_curr:
	unlock(curr->lock);
unlock_pred:
	if (pred == &set->list)
		unlock(container_of(pred, struct set_head, list)->lock);
	else
		unlock(container_of(pred, struct set_node, list)->lock);
	return retval;
}

bool remove(struct set_head *set, int elem)
{
 	int key = hash(elem);
	struct set_node *curr;
	struct list_head *pred; /* entry can be set_head or set_node */
	int retval = false;

	lock(set->lock);
	pred = &set->list;

	if (list_empty(&set->list))
		goto unlock_pred;

	list_for_each_entry(curr, &set->list, list) {
		lock(curr->lock);
		if (curr->key >= key) {
			retval = delete(curr, key);
			goto unlock_curr;
		}
		if (pred == &set->list)
			unlock(container_of(pred, struct set_head, list)->lock);
		else
			unlock(container_of(pred, struct set_node, list)->lock);
		pred = &curr->list;
	}

	/* Element not found in set */
	goto unlock_pred;

unlock_curr:
	unlock(curr->lock);
unlock_pred:
	if (pred == &set->list)
		unlock(container_of(pred, struct set_head, list)->lock);
	else
		unlock(container_of(pred, struct set_node, list)->lock);
	return retval;
}

#endif /* __FINE_SET_H__ */

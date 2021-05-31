/* A naive sequential set implementation */

#include <stdbool.h>
#include "list.h"

/* API the client should implement */
struct set_node *new_node(int key, int elem);
void free_node(struct set_node *node);

/* Set definition */
#define hash(x) (x)

struct set_node {
	int key;
	int val;
	struct list_head list;
};

struct set_head {
	struct list_head list;
};

void set_init(struct set_head *set)
{
	INIT_LIST_HEAD(&set->list);
}

bool set_insert(struct set_node *curr, int key, int elem)
{
	if (key == curr->key)
		return false;

	struct set_node *node = new_node(key, elem);
	list_add_tail(&node->list, &curr->list);
	return true;
}

bool set_delete(struct set_node *curr, int key)
{
	if (key != curr->key)
		return false;

	list_del(&curr->list);
	return true;
}

bool set_contains(struct set_head *set, int elem)
{
	int key = hash(elem);
	struct set_node *curr;

	list_for_each_entry(curr, &set->list, list) {
		if (curr->key == key)
			return true;
		if (curr->key > key)
			return false;
	}
	return false;
}

bool set_add(struct set_head *set, int elem)
{
	int key = hash(elem);
	struct set_node *curr;

	list_for_each_entry(curr, &set->list, list) {
		if (curr->key >= key)
			return set_insert(curr, key, elem);
	}
	struct set_node *node = new_node(key, elem);
	list_add_tail(&node->list, &set->list);
	return true;
}

bool set_remove(struct set_head *set, int elem)
{
	int key = hash(elem);
	struct set_node *curr;

	list_for_each_entry(curr, &set->list, list) {
		if (curr->key >= key)
			return set_delete(curr, key);
	}
	return false;
}

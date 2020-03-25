#include "list.h"

/* API the driver should implement */
struct set_node *new_node(int key, int elem);
void free_node(struct set_node *node);

/* Set definition */
#define lock(l)   pthread_mutex_lock(&(l))
#define unlock(l) pthread_mutex_unlock(&(l))

#define hash(x) (x)

struct set_node {
	int key;
	int val;
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
	int retval = false;

	lock(set->lock);
	list_for_each_entry(curr, &set->list, list) {
		if (curr->key == key) {
			retval = true;
			break;
		}
		if (curr->key > key)
			break;
	}

	unlock(set->lock);
	return retval;
}

bool add(struct set_head *set, int elem)
{
	int key = hash(elem);
	struct set_node *curr;
	int retval = true;

	lock(set->lock);
	list_for_each_entry(curr, &set->list, list) {
		if (curr->key >= key) {
			retval = insert(curr, key, elem);
			goto unlock;
		}
	}
	struct set_node *node = new_node(key, elem);
	list_add_tail(&node->list, &set->list);

unlock:
	unlock(set->lock);
	return retval;
}

bool remove(struct set_head *set, int elem)
{
	int key = hash(elem);
	struct set_node *curr;
	int retval = false;

	lock(set->lock);
	list_for_each_entry(curr, &set->list, list) {
		if (curr->key >= key) {
			retval = delete(curr, key);
			goto unlock;
		}
	}

unlock:
	unlock(set->lock);
	return retval;
}

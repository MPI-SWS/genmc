#ifndef __FINE_BST_H__
#define __FINE_BST_H__

#include <stdbool.h>

int get_thread_num();

struct bst_node *new_node(int elem);
void free_node(struct bst_node *node);

/* BST implementation */
/* NOTE: For fine-BST these require the address of the lock */
#define LOCK_TYPE pthread_mutex_t

#define LOCK(l)   pthread_mutex_lock(l)
#define UNLOCK(l) pthread_mutex_unlock(l)

#define BUG_ON(x) assert(!(x))

struct bst_node {
	int val;
	struct bst_node *left;
	struct bst_node *right;
	LOCK_TYPE lock;
};

struct bst_root {
	struct bst_node *root;
	LOCK_TYPE lock;
};

#define __BST_INITIALIZER()				\
	{ .root = NULL					\
	, .lock = PTHREAD_MUTEX_INITIALIZER }

#define DEFINE_BST(name)				\
	struct bst_root name = __BST_INITIALIZER();

/* PRE: lock_p is the address of the parent's lock, which must be held */
bool insert(struct bst_node **curr, LOCK_TYPE *lock_p, int val)
{
	BUG_ON(curr == NULL);
	if (*curr == NULL) {
		*curr = new_node(val);
		UNLOCK(lock_p);
		return true;
	}

	LOCK(&(*curr)->lock);
	UNLOCK(lock_p);

	if (val < (*curr)->val)
		return insert(&(*curr)->left, &(*curr)->lock, val);
	else if (val > (*curr)->val)
		return insert(&(*curr)->right, &(*curr)->lock, val);

	UNLOCK(&(*curr)->lock);
	return false;
}

bool add(struct bst_root *bst, int val)
{
	LOCK(&bst->lock);
	return insert(&bst->root, &bst->lock, val);
}

bool search(struct bst_node *curr, LOCK_TYPE *lock_p, int val)
{
	if (curr == NULL) {
		UNLOCK(lock_p);
		return false;
	}

	LOCK(&curr->lock);
	UNLOCK(lock_p);

	if (curr->val == val) {
		UNLOCK(&curr->lock);
		return true;
	}
	if (curr->val < val)
		return search(curr->right, &curr->lock, val);
	return search(curr->left, &curr->lock, val);
}

bool contains(struct bst_root *bst, int val)
{
	LOCK(&bst->lock);
	return search(bst->root, &bst->lock, val);
}

bool delete(struct bst_node **curr, LOCK_TYPE *lock_p, int val)
{
	BUG_ON(curr == NULL);
	if (*curr == NULL) {
		UNLOCK(lock_p);
		return false;
	}

	LOCK(&(*curr)->lock);

	if (val < (*curr)->val) {
		UNLOCK(lock_p);
		return delete(&(*curr)->left, &(*curr)->lock, val);
	}
	if (val > (*curr)->val) {
		UNLOCK(lock_p);
		return delete(&(*curr)->right, &(*curr)->lock, val);
	}

	/* Found the node to delete */
	if ((*curr)->left == NULL || (*curr)->right == NULL) {
		struct bst_node *prev_curr = *curr;
		*curr = ((*curr)->left == NULL) ? (*curr)->right : (*curr)->left;
		UNLOCK(&prev_curr->lock);
		UNLOCK(lock_p);
		free_node(prev_curr);
		return true;
	}

	struct bst_node *succ   = (*curr)->right;
	struct bst_node *succ_p = succ;

	LOCK(&succ->lock);
	while (succ->left != NULL) {
		if (succ_p != succ)
			UNLOCK(&succ_p->lock);
		succ_p = succ;
		succ = succ->left;
		LOCK(&succ->lock);
	}

	/* Check if succ_p == succ */
	if (succ_p == succ) {
		(*curr)->right = succ->right;
	} else {
		succ_p->left = succ->right;
		UNLOCK(&succ_p->lock);
	}
	(*curr)->val = succ->val;
	UNLOCK(&succ->lock);
	UNLOCK(&(*curr)->lock);
	UNLOCK(lock_p);
	free_node(succ);
	return true;
}

bool remove(struct bst_root *bst, int val)
{
	int retval = true;

	LOCK(&bst->lock);
	retval = delete(&bst->root, &bst->lock, val);
	return retval;
}

void inorder(struct bst_node *node)
{
	if (node == NULL)
		return;

	inorder(node->left);
	printf("%d\n", node->val);
	inorder(node->right);
}

void traverse(struct bst_root *bst)
{
	LOCK(&bst->lock);
	inorder(bst->root);
	UNLOCK(&bst->lock);
}

#endif /* __FINE_BST_H__ */

#ifndef __COARSE_BST_H__
#define __COARSE_BST_H__

#include <stdbool.h>

int get_thread_num();

struct bst_node *new_node(int elem);
void free_node(struct bst_node *node);

/* BST implementation */
#define LOCK(l)   pthread_mutex_lock(&(l))
#define UNLOCK(l) pthread_mutex_unlock(&(l))

#define BUG_ON(x) assert(!(x))

struct bst_node {
	int val;
	struct bst_node *left;
	struct bst_node *right;
};

struct bst_root {
	struct bst_node *root;
	pthread_mutex_t lock;
};

#define __BST_INITIALIZER()				\
	{ .root = NULL					\
	, .lock = PTHREAD_MUTEX_INITIALIZER }

#define DEFINE_BST(name)				\
	struct bst_root name = __BST_INITIALIZER();

bool insert(struct bst_node **curr, int val)
{
	BUG_ON(curr == NULL);
	if (*curr == NULL) {
		*curr = new_node(val);
		return true;
	}

	if (val < (*curr)->val)
		return insert(&(*curr)->left, val);
	else if (val > (*curr)->val)
		return insert(&(*curr)->right, val);
	return false;
}

bool add(struct bst_root *bst, int val)
{
	int retval;

	LOCK(bst->lock);
	retval = insert(&bst->root, val);
	UNLOCK(bst->lock);
	return retval;
}

bool search(struct bst_node *curr, int val)
{
	if (curr == NULL)
		return false;

	if (curr->val == val)
		return true;
	if (curr->val < val)
		return search(curr->right, val);
	return search(curr->left, val);
}

bool contains(struct bst_root *bst, int val)
{
	int retval;

	LOCK(bst->lock);
	retval = search(bst->root, val);
	UNLOCK(bst->lock);
	return retval;
}

bool delete(struct bst_node **curr, int val)
{
	BUG_ON(curr == NULL);
	if (*curr == NULL)
		return false;

	if (val < (*curr)->val)
		return delete(&(*curr)->left, val);
	if (val > (*curr)->val)
		return delete(&(*curr)->right, val);

	/* Found the node to delete */
	if ((*curr)->left == NULL) {
		struct bst_node *new_curr = (*curr)->right;
		free_node(*curr);
		*curr = new_curr;
		return true;
	}
	if ((*curr)->right == NULL) {
		struct bst_node *new_curr = (*curr)->left;
		free_node(*curr);
		*curr = new_curr;
		return true;
	}

	struct bst_node *succ   = (*curr)->right;
	struct bst_node *succ_p = (*curr)->right;

	while (succ->left != NULL) {
		succ_p = succ;
		succ = succ->left;
	}

	/* Check if succ_p == succ */
	if (succ_p == succ)
		(*curr)->right = succ->right;
	else
		succ_p->left = succ->right;
	(*curr)->val = succ->val;
	free_node(succ);
	return true;
}

bool remove(struct bst_root *bst, int val)
{
	int retval = true;

	LOCK(bst->lock);
	retval = delete(&bst->root, val);
	UNLOCK(bst->lock);
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
	LOCK(bst->lock);
	inorder(bst->root);
	UNLOCK(bst->lock);
}

#endif /* __COARSE_BST_H__ */

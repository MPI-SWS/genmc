/*
 * Adapted from: https://github.com/schets/c_flat_combining
 *
 * Slightly different from what is described in the paper:
 *     http://people.csail.mit.edu/shanir/publications/Flat%20Combining%20SPAA%2010.pdf
 *
 * since it suffers from a lot more cache contention, but offers the possibility of
 * performing asynchronous callbacks.
 */

#ifndef __COMBINER_H__
#define __COMBINER_H__

struct combine_message;

/** This is the function type which is called inside the combiner */
typedef void (*combine_fn)(struct combine_message *msg);

/** This struct holds the flat-combining lock */
struct combiner {
	struct message_metadata *queue;
	struct message_metadata *takeover;
};

struct message_metadata {
	struct message_metadata *next;
	struct message_metadata *prev;
	struct message_metadata *is_done;
	int blocking_status;
};

/** This struct describes the actions to perform when inside the lock */
struct combine_message {

	/** Function to call when inside the lock */
	combine_fn operation;

	/** Address of data to prefetch. Can be null */
	void *prefetch;

	/** Metadata used by the lock, DO NOT TOUCH */
	struct message_metadata _meta;
};

/** Performs the actions described in the msg struct, blocks until completion */
void message_combiner(struct combiner *lck, struct combine_message *msg);

/**
 * Performs the actions described in the msg struct, but does not block.
 * Instead, the message can be queried for status and blocked on until
 * completion
 * Be wary of cache contention on the stack when using this
 */
void async_message_combiner(struct combiner *lck, struct combine_message *msg);

/** Returns the status of an asynchronous message */
int async_message_status(struct combine_message *msg);

/** Blocks on an async message tag until completion */
void complete_async_message(struct combiner *cmb, struct combine_message *msg);

void init_combiner(struct combiner *cmb);

#endif /* __COMBINER_H__ */

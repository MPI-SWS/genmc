/**
 * Empty specification just to be able to fully compile mpc.c, so we can link in our Rust-Code afterwards.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef SEGQUEUE
typedef struct {
    uintptr_t index;
    void* block;
} position_t;

typedef struct {
    position_t head;
    position_t tail;
} seg_queue_t;

typedef seg_queue_t queue_t;
#endif

#ifdef ARRAYQUEUE
typedef struct {
    uintptr_t head;
    uintptr_t tail;
    void* buffer1;
    void* buffer2;
    uint64_t one_lap;
} array_queue_t;

typedef array_queue_t queue_t;
#endif

#ifdef MSQUEUE
typedef struct {
	unsigned int value;
	_Atomic(struct ms_node_t *) next;
} ms_node_t;

typedef struct {
	_Atomic(ms_node_t *) head;
	_Atomic(ms_node_t *) tail;
} ms_queue_t;

typedef ms_node_t node_t;
typedef ms_queue_t queue_t;
#endif

extern void init_queue(queue_t *q, int num_thread) __attribute__ ((__nothrow__));

extern void clear_queue(queue_t *q, int num_thread) __attribute__ ((__nothrow__));

extern void enqueue(queue_t *q, unsigned int val) __attribute__ ((__nothrow__));

extern bool dequeue(queue_t *q, unsigned int *ret) __attribute__ ((__nothrow__));

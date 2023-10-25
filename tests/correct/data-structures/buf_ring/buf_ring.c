/*******************************************************************************
 * Tests with FreeBSD 8.0.0 buf_ring.h
 * The buf_ring has been slightly modified, this is only for testing GenMC.
 * See problems below.
 ******************************************************************************/

//#define DEQUEUE_NONATOMIC
//#define ENQUEUE_NONATOMIC
//#define DISABLE_ENQUEUE_BARRIER
//#define DISABLE_DEQUEUE_BARRIER

/*******************************************************************************
 * config
 ******************************************************************************/
#ifndef NTHREADS
# define NTHREADS 3
#endif
#define LOOPS 1
#define BUF_SIZE 8
#define SPIN_ANNOTATION

#define caddr_t char*
#define atomic_cmpset_int(dst, expect, src) \
        __atomic_compare_exchange_n(dst, &expect, src, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQ_REL)
#define cpu_spinwait() __VERIFIER_assume(0)

#define await_while while

/******************************************************************************
 * Copyright (c) 2007,2008 Kip Macy kmacy@freebsd.org
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. The name of Kip Macy nor the names of other
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

struct buf_ring {
	volatile uint32_t	br_prod_head;
	volatile uint32_t	br_prod_tail;
	int              	br_prod_size;
	int              	br_prod_mask;
	volatile uint32_t	br_cons_head;
	volatile uint32_t	br_cons_tail;
	int		 	br_cons_size;
	int              	br_cons_mask;
	void			*br_ring[0];
};

static __inline int
buf_ring_enqueue_bytes(struct buf_ring *br, void *buf, int nbytes)
{
	uint32_t prod_head, prod_next;
	uint32_t cons_tail;
	int success;

	do {
		prod_head = br->br_prod_head;
		cons_tail = br->br_cons_tail;
		prod_next = (prod_head + 1) & br->br_prod_mask;
		if (prod_next == cons_tail) return (-1);
		success = atomic_cmpset_int(&br->br_prod_head, prod_head,
		    prod_next);
		if (!success) cpu_spinwait();
	} while (success == 0);

#ifndef DISABLE_ENQUEUE_BARRIER
	__sync_synchronize();
#endif

#ifdef ENQUEUE_NONATOMIC
	br->br_ring[prod_head] = buf;
#else
	__atomic_store_n(&br->br_ring[prod_head], buf, __ATOMIC_RELAXED);
#endif
	await_while (br->br_prod_tail != prod_head) cpu_spinwait();

	br->br_prod_tail = prod_next;

	return (0);
}

static __inline int
buf_ring_enqueue(struct buf_ring *br, void *buf)
{

	return (buf_ring_enqueue_bytes(br, buf, 0));
}

static __inline void *
buf_ring_dequeue_mc(struct buf_ring *br)
{
	uint32_t cons_head, cons_next;
	uint32_t prod_tail;
	void *buf;
	int success;

	do {
		cons_head = br->br_cons_head;
		prod_tail = br->br_prod_tail;
		cons_next = (cons_head + 1) & br->br_cons_mask;
		if (cons_head == prod_tail) return (NULL);
		success = atomic_cmpset_int(&br->br_cons_head, cons_head,
		    cons_next);
		if (!success) cpu_spinwait();
	} while (success == 0);

#ifdef DEQUEUE_NONATOMIC
	buf = br->br_ring[cons_head];
#else
	buf = __atomic_load_n(&br->br_ring[cons_head], __ATOMIC_RELAXED);
#endif
	await_while (br->br_cons_tail != cons_head) cpu_spinwait();

#ifndef DISABLE_DEQUEUE_BARRIER
	__sync_synchronize();
#endif
	br->br_cons_tail = cons_next;

	return (buf);
}

struct buf_ring *
buf_ring_alloc(int count)
{
	struct buf_ring *br;
	br = malloc(sizeof(struct buf_ring) + count*sizeof(caddr_t));
	if (br == NULL) return NULL;
	br->br_prod_size = br->br_cons_size = count;
	br->br_prod_mask = br->br_cons_mask = count-1;
	br->br_prod_head = br->br_cons_head = 0;
	br->br_prod_tail = br->br_cons_tail = 0;
	for (int i = 0; i < count; i++) br->br_ring[i] = 0;
	return br;
}
#define buf_ring_free(br, X) free(br)

/*******************************************************************************
 * test case
 ******************************************************************************/
struct buf_ring *br;

static void *run(void *arg) {
	uint32_t tid = (intptr_t)arg;
	int *messages = malloc(sizeof(int) * LOOPS);

	for (int i = 0; i < LOOPS; i++) {
		int *buf = &messages[i];
		*buf = 0xBEEF;
		if (buf_ring_enqueue(br, buf) != 0)
			*buf = 0xDEAD;

		if ((buf = buf_ring_dequeue_mc(br)) != NULL)
			assert (*buf == 0xBEEF);
	}
	return NULL;
}

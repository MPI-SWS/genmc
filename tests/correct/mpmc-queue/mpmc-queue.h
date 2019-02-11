#include <unistd.h>
#include <inttypes.h>
#include <stdatomic.h>
#include "ordering.h"

#define t_size sizeof(int32_t)

typedef int32_t t_element;


struct mpmc_boundq_1_alt
{

	// elements should generally be cache-line-size padded :
	t_element               m_array[t_size];

	// rdwr counts the reads & writes that have started
	atomic_uint    m_rdwr;
	// "read" and "written" count the number completed
	atomic_uint    m_read;
	atomic_uint    m_written;
};
typedef struct mpmc_boundq_1_alt mpmc_boundq_1_alt;

t_element *read_fetch(mpmc_boundq_1_alt *q)
{
	unsigned int rdwr = atomic_load_explicit(&q->m_rdwr, mo_acquire);
	unsigned int rd,wr;

	for(;;) {
		rd = (rdwr >> 16) & 0xFFFF;
		wr = rdwr & 0xFFFF;

		if (wr == rd) // empty
			return NULL;

		if (atomic_compare_exchange_weak_explicit(&q->m_rdwr, &rdwr, rdwr + (1 << 16),
							   mo_acq_rel, mo_acq_rel))
			break;
	}

	while ((atomic_load_explicit(&q->m_written, mo_acquire) & 0xFFFF) != wr)
		; // thrd_yield();

	t_element *p = &(q->m_array[rd % t_size]);
	return p;
}

void read_consume(mpmc_boundq_1_alt *q)
{
	atomic_fetch_add_explicit(&q->m_read, 1, mo_release);
}

t_element *write_prepare(mpmc_boundq_1_alt *q)
{
	unsigned int rdwr = atomic_load_explicit(&q->m_rdwr, mo_acquire);
	unsigned int rd,wr;

	for(;;) {
		rd = (rdwr>>16) & 0xFFFF;
		wr = rdwr & 0xFFFF;

		if (wr == ((rd + t_size) & 0xFFFF)) // full
			return NULL;

		if (atomic_compare_exchange_weak_explicit(&q->m_rdwr, &rdwr, (rd << 16) | ((wr + 1) & 0xFFFF),
							  mo_acq_rel, mo_acq_rel))
			break;
	}

	while ((atomic_load_explicit(&q->m_read, mo_acquire) & 0xFFFF) != rd )
		; // thrd_yield()

	t_element *p = &(q->m_array[wr % t_size]);
	return p;
}

void write_publish(mpmc_boundq_1_alt *q)
{
	atomic_fetch_add_explicit(&q->m_written, 1, mo_release);
}

atomic_int lock;

void *threadRa(void *arg)
{
        atomic_fetch_add_explicit(&lock, 1, memory_order_acquire);
        atomic_fetch_add_explicit(&lock, 1, memory_order_relaxed);
        return NULL;
}

void *threadR(void *arg)
{
        atomic_fetch_add_explicit(&lock, 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&lock, 1, memory_order_relaxed);
        return NULL;
}

void *threadRr(void *arg)
{
        atomic_fetch_add_explicit(&lock, 1, memory_order_release);
        return NULL;
}

void *threadRs(void *arg)
{
        atomic_fetch_add_explicit(&lock, 1, memory_order_relaxed);
        return NULL;
}

#pragma once

#define CAS(...) atomic_compare_exchange_strong_explicit(__VA_ARGS__)
#define load(...) atomic_load_explicit(__VA_ARGS__)
#define store(...) atomic_store_explicit(__VA_ARGS__)
#define FAA(...) atomic_fetch_add_explicit(__VA_ARGS__)
#define XCHG(...) atomic_exchange_explicit(__VA_ARGS__)

#define relaxed memory_order_relaxed
#define release memory_order_release
#define acquire memory_order_acquire
#define seq_cst memory_order_seq_cst
#define acq_rel memory_order_acq_rel
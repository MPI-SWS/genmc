#ifdef MAKE_ALL_SC
# define mo_rlx memory_order_seq_cst
# define mo_acq memory_order_seq_cst
# define mo_rel memory_order_seq_cst
# define mo_acq_rel memory_order_seq_cst
# define mo_seq_cst memory_order_seq_cst
#else
# define mo_rlx memory_order_relaxed
# define mo_acq memory_order_acquire
# define mo_rel memory_order_release
# define mo_acq_rel memory_order_acq_rel
# define mo_seq_cst memory_order_seq_cst
#endif

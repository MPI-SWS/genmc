#include <genmc.h>

#define PRELUDE_DECL() \
queue_t queue;

#define MAIN_THREAD_BODY_INIT() \
   __VERIFIER_method_begin("init_queue", 0); \
   init_queue(&queue, NUM_THREADS);    \
   __VERIFIER_method_end("init_queue", 0);

// NB: this allows to avoid warning about not freed memory
//  in the end of execution, but it requires joining all thread before.
//  Meanwhile joining thread creates HB with main thread and
//  contradict current definition of the Most-Parallel-Client.
//  So it's unused for now.
#define MAIN_THREAD_BODY_FINISH() \
   __VERIFIER_method_begin("clear_queue", 0); \
   clear_queue(&queue, NUM_THREADS);      \
   __VERIFIER_method_end("clear_queue", 0);

#define THREAD_BODY_1()                 \
   unsigned int value = 100 + get_thread_num();  \
   __VERIFIER_method_begin("enqueue", value);  \
   enqueue(&queue, value);              \
   __VERIFIER_method_end("enqueue", 0);

#define THREAD_BODY_2()    \
   unsigned int ret;            \
   __VERIFIER_method_begin("dequeue", 0);    \
   bool succ = dequeue(&queue, &ret);                 \
   __VERIFIER_method_end("dequeue", succ ? ret : 0);

// redefine parameters
#ifdef RTN
# define THREAD_NUM_1 WTN
#else
# define THREAD_NUM_1 1
#endif
#ifdef WTN
# define THREAD_NUM_2 RTN
#else
# define THREAD_NUM_2 1
#endif

#include "../mpc_template.h"

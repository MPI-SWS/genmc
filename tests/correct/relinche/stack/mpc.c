#include <genmc.h>

#define PRELUDE_DECL() mystack_t s;

#define MAIN_THREAD_BODY_INIT() \
  __VERIFIER_method_begin("init_stack", 0); \
  init_stack(&s, NUM_THREADS); \
  __VERIFIER_method_end("init_stack", 0);

#define MAIN_THREAD_BODY_FINISH() \
  __VERIFIER_method_begin("clear_stack", 0); \
  clear_stack(&s, NUM_THREADS);   \
  __VERIFIER_method_end("clear_stack", 0);

#define THREAD_BODY_1() \
  unsigned int value = 100 + get_thread_num(); \
  __VERIFIER_method_begin("push", value);  \
  push(&s, value); \
  __VERIFIER_method_end("push", 0);

#define THREAD_BODY_2() \
  unsigned int ret;              \
  __VERIFIER_method_begin("pop", 0); \
  ret = pop(&s);                  \
  __VERIFIER_method_end("pop", ret);

#ifndef RTN
#define RTN 1
#endif
#ifndef WTN
#define WTN 1
#endif
#define THREAD_NUM_1 WTN
#define THREAD_NUM_2 RTN


#include "../mpc_template.h"

#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <genmc_internal.h>

#undef assert
#undef __assert

#ifdef __cplusplus
extern "C" {
#endif

#define assert(e)  \
    ((void) ((e) ? ((void)0) : __VERIFIER_assert_fail (#e, __FILE__, __LINE__)))

#ifdef __cplusplus
}
#endif

#endif /* __ASSERT_H__ */

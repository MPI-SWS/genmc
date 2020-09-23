#undef assert
#undef __assert

#ifdef __cplusplus
extern "C" {
#endif

#define assert(e)  \
    ((void) ((e) ? ((void)0) : __VERIFIER_assert_fail (#e, __FILE__, __LINE__)))

extern void __VERIFIER_assert_fail(const char *, const char *, int);

#ifdef __cplusplus
}
#endif

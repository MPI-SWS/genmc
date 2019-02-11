#undef assert
#undef __assert

#define assert(e)  \
    ((void) ((e) ? ((void)0) : __assert_fail (#e, __FILE__, __LINE__)))

extern void __assert_fail(const char *, const char *, int);


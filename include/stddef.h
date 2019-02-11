#ifndef __STDDEF_H__
#define __STDDEF_H__

typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __SIZE_TYPE__ size_t;
typedef __WCHAR_TYPE__ wchar_t;

#undef NULL
#define NULL ((void*)0)

#endif

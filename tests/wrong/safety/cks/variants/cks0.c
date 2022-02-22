/*
 * Copyright 2011-2015 Samy Al Bahra.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <pthread.h>
#include <assert.h>
#include <stdint.h>

# 1 "../../../include/ck_sequence.h" 1
# 30 "../../../include/ck_sequence.h"
# 1 "../../../include/ck_cc.h" 1
# 32 "../../../include/ck_cc.h"
# 1 "../../../include/gcc/ck_cc.h" 1
# 31 "../../../include/gcc/ck_cc.h"
# 1 "../../../include/ck_md.h" 1
# 32 "../../../include/gcc/ck_cc.h" 2
# 119 "../../../include/gcc/ck_cc.h"
__attribute__((unused)) static int
ck_cc_ffs(unsigned int x)
{

 return __builtin_ffsl(x);
}


__attribute__((unused)) static int
ck_cc_ffsl(unsigned long x)
{

 return __builtin_ffsll(x);
}


__attribute__((unused)) static int
ck_cc_ctz(unsigned int x)
{

 return __builtin_ctz(x);
}


__attribute__((unused)) static int
ck_cc_popcount(unsigned int x)
{

 return __builtin_popcount(x);
}
# 33 "../../../include/ck_cc.h" 2
# 134 "../../../include/ck_cc.h"
__attribute__((unused)) static int ck_cc_ffsll(unsigned long long v) { unsigned int i; if (v == 0) return 0; for (i = 1; (v & 1) == 0; i++, v >>= 1); return i; }
# 31 "../../../include/ck_sequence.h" 2
# 1 "../../../include/ck_pr.h" 1
# 32 "../../../include/ck_pr.h"
# 1 "../../../include/ck_limits.h" 1
# 47 "../../../include/ck_limits.h"
# 1 "/usr/lib/llvm-8/lib/clang/8.0.0/include/limits.h" 1 3
# 37 "/usr/lib/llvm-8/lib/clang/8.0.0/include/limits.h" 3
# 1 "/usr/include/limits.h" 1 3 4
# 26 "/usr/include/limits.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/limits.h" 2 3 4
# 183 "/usr/include/limits.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 1 3 4
# 160 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 1 3 4
# 38 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 3 4
# 1 "/usr/include/linux/limits.h" 1 3 4
# 39 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 2 3 4
# 161 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 2 3 4
# 184 "/usr/include/limits.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/posix2_lim.h" 1 3 4
# 188 "/usr/include/limits.h" 2 3 4
# 38 "/usr/lib/llvm-8/lib/clang/8.0.0/include/limits.h" 2 3
# 48 "../../../include/ck_limits.h" 2
# 33 "../../../include/ck_pr.h" 2

# 1 "../../../include/ck_stdint.h" 1
# 33 "../../../include/ck_stdint.h"
# 1 "/usr/lib/llvm-8/lib/clang/8.0.0/include/stdint.h" 1 3
# 61 "/usr/lib/llvm-8/lib/clang/8.0.0/include/stdint.h" 3
# 1 "/usr/include/stdint.h" 1 3 4
# 26 "/usr/include/stdint.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/stdint.h" 2 3 4

# 1 "/usr/include/x86_64-linux-gnu/bits/wchar.h" 1 3 4
# 29 "/usr/include/stdint.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 30 "/usr/include/stdint.h" 2 3 4






# 34 "../../../include/ck_stdint.h" 2
# 35 "../../../include/ck_pr.h" 2
# 1 "../../../include/ck_stdbool.h" 1
# 30 "../../../include/ck_stdbool.h"
# 1 "/usr/lib/llvm-8/lib/clang/8.0.0/include/stdbool.h" 1 3
# 31 "../../../include/ck_stdbool.h" 2
# 36 "../../../include/ck_pr.h" 2
# 73 "../../../include/ck_pr.h"
# 1 "../../../include/gcc/ck_pr.h" 1
# 36 "../../../include/gcc/ck_pr.h"
__attribute__((unused)) static void
ck_pr_barrier(void){}





# 1 "../../../include/ck_stdbool.h" 1
# 48 "../../../include/gcc/ck_pr.h" 2
# 1 "../../../include/ck_stdint.h" 1
# 33 "../../../include/ck_stdint.h"
# 1 "/usr/lib/llvm-8/lib/clang/8.0.0/include/stdint.h" 1 3
# 34 "../../../include/ck_stdint.h" 2
# 49 "../../../include/gcc/ck_pr.h" 2





# 1 "../../../include/gcc/ck_f_pr.h" 1
# 55 "../../../include/gcc/ck_pr.h" 2
# 77 "../../../include/gcc/ck_pr.h"
__attribute__((unused)) static void *
ck_pr_md_load_ptr(const void *target)
{
 void *r;

 ck_pr_barrier();
 r = ((void *)(uintptr_t)(*(volatile void *const*)(target)));
 ck_pr_barrier();

 return r;
}

__attribute__((unused)) static void
ck_pr_md_store_ptr(void *target, const void *v)
{

 ck_pr_barrier();
 *(volatile void **)target = ((void *)(uintptr_t)(v));
 ck_pr_barrier();
 return;
}



__attribute__((unused)) static char ck_pr_md_load_char(const char *target) { char r; ck_pr_barrier(); r = (*(volatile __typeof__(*(const char *)target) *)&(*(const char *)target)); ck_pr_barrier(); return (r); } __attribute__((unused)) static void ck_pr_md_store_char(char *target, char v) { ck_pr_barrier(); (*(volatile __typeof__(*(char *)target) *)&(*(char *)target)) = v; ck_pr_barrier(); return; }
__attribute__((unused)) static unsigned int ck_pr_md_load_uint(const unsigned int *target) { unsigned int r; ck_pr_barrier(); r = (*(volatile __typeof__(*(const unsigned int *)target) *)&(*(const unsigned int *)target)); ck_pr_barrier(); return (r); } __attribute__((unused))static void ck_pr_md_store_uint(unsigned int *target, unsigned int v) { ck_pr_barrier(); (*(volatile __typeof__(*(unsigned int *)target) *)&(*(unsigned int *)target)) = v; ck_pr_barrier(); return; }
__attribute__((unused)) static int ck_pr_md_load_int(const int *target) { int r; ck_pr_barrier(); r = (*(volatile __typeof__(*(const int *)target) *)&(*(const int *)target)); ck_pr_barrier(); return (r); } __attribute__((unused)) static void ck_pr_md_store_int(int *target, int v) { ck_pr_barrier(); (*(volatile __typeof__(*(int *)target) *)&(*(int *)target)) = v; ck_pr_barrier(); return; }

__attribute__((unused)) static double ck_pr_md_load_double(const double *target) { double r; ck_pr_barrier(); r = (*(volatile __typeof__(*(const double *)target) *)&(*(const double *)target)); ck_pr_barrier(); return (r); } __attribute__((unused)) static void ck_pr_md_store_double(double *target, double v) { ck_pr_barrier(); (*(volatile __typeof__(*(double *)target) *)&(*(double *)target)) = v; ck_pr_barrier(); return; }

__attribute__((unused)) static uint64_t ck_pr_md_load_64(const uint64_t *target) { uint64_t r; ck_pr_barrier(); r = (*(volatile __typeof__(*(const uint64_t *)target) *)&(*(const uint64_t *)target)); ck_pr_barrier(); return (r); } __attribute__((unused)) static void ck_pr_md_store_64(uint64_t *target, uint64_t v) { ck_pr_barrier(); (*(volatile __typeof__(*(uint64_t *)target) *)&(*(uint64_t *)target)) = v; ck_pr_barrier(); return; }
__attribute__((unused)) static uint32_t ck_pr_md_load_32(const uint32_t *target) { uint32_t r; ck_pr_barrier(); r = (*(volatile __typeof__(*(const uint32_t *)target) *)&(*(const uint32_t *)target)); ck_pr_barrier(); return (r); } __attribute__((unused)) static void ck_pr_md_store_32(uint32_t *target, uint32_t v) { ck_pr_barrier(); (*(volatile __typeof__(*(uint32_t *)target) *)&(*(uint32_t *)target)) = v; ck_pr_barrier(); return; }
__attribute__((unused)) static uint16_t ck_pr_md_load_16(const uint16_t *target) { uint16_t r; ck_pr_barrier(); r = (*(volatile __typeof__(*(const uint16_t *)target) *)&(*(const uint16_t *)target)); ck_pr_barrier(); return (r); } __attribute__((unused)) static void ck_pr_md_store_16(uint16_t *target, uint16_t v) { ck_pr_barrier(); (*(volatile __typeof__(*(uint16_t *)target) *)&(*(uint16_t *)target)) = v; ck_pr_barrier(); return; }
__attribute__((unused)) static uint8_t ck_pr_md_load_8(const uint8_t *target) { uint8_t r; ck_pr_barrier(); r = (*(volatile __typeof__(*(const uint8_t *)target) *)&(*(const uint8_t *)target)); ck_pr_barrier(); return (r); } __attribute__((unused)) static void ck_pr_md_store_8(uint8_t *target, uint8_t v) { ck_pr_barrier(); (*(volatile __typeof__(*(uint8_t *)target) *)&(*(uint8_t *)target)) = v; ck_pr_barrier(); return; }




__attribute__((unused)) static void
ck_pr_stall(void)
{

 ck_pr_barrier();
}
# 132 "../../../include/gcc/ck_pr.h"
__attribute__((unused)) static void ck_pr_fence_strict_atomic(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_atomic_atomic(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_atomic_load(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_atomic_store(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_store_atomic(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_load_atomic(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_load(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_load_load(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_load_store(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_store(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_store_store(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_store_load(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_memory(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_acquire(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_release(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_acqrel(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_lock(void) { __sync_synchronize(); }
__attribute__((unused)) static void ck_pr_fence_strict_unlock(void) { __sync_synchronize(); }
# 165 "../../../include/gcc/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_cas_ptr(void *target, void * compare, void * set) { _Bool z; z = __sync_bool_compare_and_swap((void * *)target, compare, set); return z; }



__attribute__((unused)) static _Bool ck_pr_cas_char(char *target, char compare, char set) { _Bool z; z = __sync_bool_compare_and_swap((char *)target, compare, set); return z; }
__attribute__((unused)) static _Bool ck_pr_cas_int(int *target, int compare, int set) { _Bool z; z = __sync_bool_compare_and_swap((int *)target, compare, set); return z; }
__attribute__((unused)) static _Bool ck_pr_cas_uint(unsigned int *target, unsigned int compare, unsigned int set) { _Bool z; z = __sync_bool_compare_and_swap((unsigned int *)target, compare, set); return z; }
__attribute__((unused)) static _Bool ck_pr_cas_64(uint64_t *target, uint64_t compare, uint64_t set) { _Bool z; z = __sync_bool_compare_and_swap((uint64_t *)target, compare, set); return z; }
__attribute__((unused)) static _Bool ck_pr_cas_32(uint32_t *target, uint32_t compare, uint32_t set) { _Bool z; z = __sync_bool_compare_and_swap((uint32_t *)target, compare, set); return z; }
__attribute__((unused)) static _Bool ck_pr_cas_16(uint16_t *target, uint16_t compare, uint16_t set) { _Bool z; z = __sync_bool_compare_and_swap((uint16_t *)target, compare, set); return z; }
__attribute__((unused)) static _Bool ck_pr_cas_8(uint8_t *target, uint8_t compare, uint8_t set) { _Bool z; z = __sync_bool_compare_and_swap((uint8_t *)target, compare, set); return z; }







__attribute__((unused)) static _Bool
ck_pr_cas_ptr_value(void *target, void *compare, void *set, void *v)
{
 set = __sync_val_compare_and_swap((void **)target, compare, set);
 *(void **)v = set;
 return (set == compare);
}
# 200 "../../../include/gcc/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_cas_char_value(char *target, char compare, char set, char *v) { set = __sync_val_compare_and_swap(target, compare, set); *v = set; return (set == compare); }
__attribute__((unused)) static _Bool ck_pr_cas_int_value(int *target, int compare, int set, int *v) { set = __sync_val_compare_and_swap(target, compare, set); *v = set; return (set == compare); }
__attribute__((unused)) static _Bool ck_pr_cas_uint_value(unsigned int *target, unsigned int compare, unsigned int set, unsigned int *v) { set = __sync_val_compare_and_swap(target, compare, set); *v = set; return (set == compare); }
__attribute__((unused)) static _Bool ck_pr_cas_64_value(uint64_t *target, uint64_t compare, uint64_t set, uint64_t *v) { set = __sync_val_compare_and_swap(target, compare, set); *v = set; return (set == compare); }
__attribute__((unused)) static _Bool ck_pr_cas_32_value(uint32_t *target, uint32_t compare, uint32_t set, uint32_t *v) { set = __sync_val_compare_and_swap(target, compare, set); *v = set; return (set == compare); }
__attribute__((unused)) static _Bool ck_pr_cas_16_value(uint16_t *target, uint16_t compare, uint16_t set, uint16_t *v) { set = __sync_val_compare_and_swap(target, compare, set); *v = set; return (set == compare); }
__attribute__((unused)) static _Bool ck_pr_cas_8_value(uint8_t *target, uint8_t compare, uint8_t set, uint8_t *v) { set = __sync_val_compare_and_swap(target, compare, set); *v = set; return (set == compare); }
# 221 "../../../include/gcc/ck_pr.h"
__attribute__((unused)) static void * ck_pr_faa_ptr(void *target, void * d) { d = __sync_fetch_and_add((void * *)target, d); return (d); }



__attribute__((unused)) static char ck_pr_faa_char(char *target, char d) { d = __sync_fetch_and_add((char *)target, d); return (d); }
__attribute__((unused)) static unsigned int ck_pr_faa_uint(unsigned int *target, unsigned int d) { d = __sync_fetch_and_add((unsigned int *)target, d); return (d); }
__attribute__((unused)) static int ck_pr_faa_int(int *target, int d) { d = __sync_fetch_and_add((int *)target, d); return (d); }
__attribute__((unused)) static uint64_t ck_pr_faa_64(uint64_t *target, uint64_t d) { d = __sync_fetch_and_add((uint64_t *)target, d); return (d); }
__attribute__((unused)) static uint32_t ck_pr_faa_32(uint32_t *target, uint32_t d) { d = __sync_fetch_and_add((uint32_t *)target, d); return (d); }
__attribute__((unused)) static uint16_t ck_pr_faa_16(uint16_t *target, uint16_t d) { d = __sync_fetch_and_add((uint16_t *)target, d); return (d); }
__attribute__((unused)) static uint8_t ck_pr_faa_8(uint8_t *target, uint8_t d) { d = __sync_fetch_and_add((uint8_t *)target, d); return (d); }
# 259 "../../../include/gcc/ck_pr.h"
__attribute__((unused)) static void ck_pr_add_ptr(void *target, void * d) { d = __sync_fetch_and_add((void * *)target, d); return; } __attribute__((unused)) static void ck_pr_add_char(char *target, char d) { d = __sync_fetch_and_add((char *)target, d); return; } __attribute__((unused)) static void ck_pr_add_int(int *target, int d) { d = __sync_fetch_and_add((int *)target, d); return; } __attribute__((unused)) static void ck_pr_add_uint(unsigned int *target, unsigned int d) { d = __sync_fetch_and_add((unsigned int *)target, d); return; } __attribute__((unused)) static void ck_pr_add_64(uint64_t *target, uint64_t d) { d = __sync_fetch_and_add((uint64_t *)target, d); return; } __attribute__((unused)) static void ck_pr_add_32(uint32_t *target, uint32_t d) { d = __sync_fetch_and_add((uint32_t *)target, d); return; } __attribute__((unused)) static void ck_pr_add_16(uint16_t *target, uint16_t d) { d = __sync_fetch_and_add((uint16_t *)target, d); return; } __attribute__((unused)) static void ck_pr_add_8(uint8_t *target, uint8_t d) { d = __sync_fetch_and_add((uint8_t *)target, d); return; }
__attribute__((unused)) static void ck_pr_sub_ptr(void *target, void * d) { d = __sync_fetch_and_sub((void * *)target, d); return; } __attribute__((unused)) static void ck_pr_sub_char(char *target, char d) { d = __sync_fetch_and_sub((char *)target, d); return; } __attribute__((unused)) static void ck_pr_sub_int(int *target, int d) { d = __sync_fetch_and_sub((int *)target, d); return; } __attribute__((unused)) static void ck_pr_sub_uint(unsigned int *target, unsigned int d) { d = __sync_fetch_and_sub((unsigned int *)target, d); return; } __attribute__((unused)) static void ck_pr_sub_64(uint64_t *target, uint64_t d) { d = __sync_fetch_and_sub((uint64_t *)target, d); return; } __attribute__((unused)) static void ck_pr_sub_32(uint32_t *target, uint32_t d) { d = __sync_fetch_and_sub((uint32_t *)target, d); return; } __attribute__((unused)) static void ck_pr_sub_16(uint16_t *target, uint16_t d) { d = __sync_fetch_and_sub((uint16_t *)target, d); return; } __attribute__((unused)) static void ck_pr_sub_8(uint8_t *target, uint8_t d) { d = __sync_fetch_and_sub((uint8_t *)target, d); return; }
__attribute__((unused)) static void ck_pr_and_ptr(void *target, void * d) { d = __sync_fetch_and_and((void * *)target, d); return; } __attribute__((unused)) static void ck_pr_and_char(char *target, char d) { d = __sync_fetch_and_and((char *)target, d); return; } __attribute__((unused)) static void ck_pr_and_int(int *target, int d) { d = __sync_fetch_and_and((int *)target, d); return; } __attribute__((unused)) static void ck_pr_and_uint(unsigned int *target, unsigned int d) { d = __sync_fetch_and_and((unsigned int *)target, d); return; } __attribute__((unused)) static void ck_pr_and_64(uint64_t *target, uint64_t d) { d = __sync_fetch_and_and((uint64_t *)target, d); return; } __attribute__((unused)) static void ck_pr_and_32(uint32_t *target, uint32_t d) { d = __sync_fetch_and_and((uint32_t *)target, d); return; } __attribute__((unused)) static void ck_pr_and_16(uint16_t *target, uint16_t d) { d = __sync_fetch_and_and((uint16_t *)target, d); return; } __attribute__((unused)) static void ck_pr_and_8(uint8_t *target, uint8_t d) { d = __sync_fetch_and_and((uint8_t *)target, d); return; }
__attribute__((unused)) static void ck_pr_or_ptr(void *target, void * d) { d = __sync_fetch_and_or((void * *)target, d); return; } __attribute__((unused)) static void ck_pr_or_char(char *target, char d) { d = __sync_fetch_and_or((char *)target, d); return; } __attribute__((unused)) static void ck_pr_or_int(int *target, int d) { d = __sync_fetch_and_or((int *)target, d); return; } __attribute__((unused)) static void ck_pr_or_uint(unsigned int *target, unsigned int d) { d = __sync_fetch_and_or((unsigned int *)target, d); return; } __attribute__((unused)) static void ck_pr_or_64(uint64_t *target, uint64_t d) { d = __sync_fetch_and_or((uint64_t *)target, d); return; } __attribute__((unused)) static void ck_pr_or_32(uint32_t *target, uint32_t d) { d = __sync_fetch_and_or((uint32_t *)target, d); return; } __attribute__((unused)) static void ck_pr_or_16(uint16_t *target, uint16_t d) { d = __sync_fetch_and_or((uint16_t *)target, d); return; } __attribute__((unused)) static void ck_pr_or_8(uint8_t *target, uint8_t d) { d = __sync_fetch_and_or((uint8_t *)target, d); return; }
__attribute__((unused)) static void ck_pr_xor_ptr(void *target, void * d) { d = __sync_fetch_and_xor((void * *)target, d); return; } __attribute__((unused)) static void ck_pr_xor_char(char *target, char d) { d = __sync_fetch_and_xor((char *)target, d); return; } __attribute__((unused)) static void ck_pr_xor_int(int *target, int d) { d = __sync_fetch_and_xor((int *)target, d); return; } __attribute__((unused)) static void ck_pr_xor_uint(unsigned int *target, unsigned int d) { d = __sync_fetch_and_xor((unsigned int *)target, d); return; } __attribute__((unused)) static void ck_pr_xor_64(uint64_t *target, uint64_t d) { d = __sync_fetch_and_xor((uint64_t *)target, d); return; } __attribute__((unused)) static void ck_pr_xor_32(uint32_t *target, uint32_t d) { d = __sync_fetch_and_xor((uint32_t *)target, d); return; } __attribute__((unused)) static void ck_pr_xor_16(uint16_t *target, uint16_t d) { d = __sync_fetch_and_xor((uint16_t *)target, d); return; } __attribute__((unused)) static void ck_pr_xor_8(uint8_t *target, uint8_t d) { d = __sync_fetch_and_xor((uint8_t *)target, d); return; }
# 285 "../../../include/gcc/ck_pr.h"
__attribute__((unused)) static void ck_pr_inc_ptr(void *target) { ck_pr_add_ptr(target, (void *)1); return; } __attribute__((unused)) static void ck_pr_dec_ptr(void *target) { ck_pr_sub_ptr(target, (void *)1); return; }
__attribute__((unused)) static void ck_pr_inc_char(char *target) { ck_pr_add_char(target, (char)1); return; } __attribute__((unused)) static void ck_pr_dec_char(char *target) { ck_pr_sub_char(target, (char)1); return; }
__attribute__((unused)) static void ck_pr_inc_int(int *target) { ck_pr_add_int(target, (int)1); return; } __attribute__((unused)) static void ck_pr_dec_int(int *target) { ck_pr_sub_int(target, (int)1); return; }
__attribute__((unused)) static void ck_pr_inc_uint(unsigned int *target) { ck_pr_add_uint(target, (unsigned int)1); return; } __attribute__((unused)) static void ck_pr_dec_uint(unsigned int *target) { ck_pr_sub_uint(target, (unsigned int)1); return; }
__attribute__((unused)) static void ck_pr_inc_64(uint64_t *target) { ck_pr_add_64(target, (uint64_t)1); return; } __attribute__((unused)) static void ck_pr_dec_64(uint64_t *target) { ck_pr_sub_64(target, (uint64_t)1); return; }
__attribute__((unused)) static void ck_pr_inc_32(uint32_t *target) { ck_pr_add_32(target, (uint32_t)1); return; } __attribute__((unused)) static void ck_pr_dec_32(uint32_t *target) { ck_pr_sub_32(target, (uint32_t)1); return; }
__attribute__((unused)) static void ck_pr_inc_16(uint16_t *target) { ck_pr_add_16(target, (uint16_t)1); return; } __attribute__((unused)) static void ck_pr_dec_16(uint16_t *target) { ck_pr_sub_16(target, (uint16_t)1); return; }
__attribute__((unused)) static void ck_pr_inc_8(uint8_t *target) { ck_pr_add_8(target, (uint8_t)1); return; } __attribute__((unused)) static void ck_pr_dec_8(uint8_t *target) { ck_pr_sub_8(target, (uint8_t)1); return; }
# 74 "../../../include/ck_pr.h" 2
# 95 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_fence_load_depends(void) { ck_pr_barrier(); return; }
# 147 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_fence_atomic(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_atomic_load(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_atomic_store(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_store_atomic(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_load_atomic(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_load_store(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_store_load(void) { ck_pr_fence_strict_store_load(); return; }
__attribute__((unused)) static void ck_pr_fence_load(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_store(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_memory(void) { ck_pr_fence_strict_memory(); return; }
__attribute__((unused)) static void ck_pr_fence_acquire(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_release(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_acqrel(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_lock(void) { ck_pr_barrier(); return; }
__attribute__((unused)) static void ck_pr_fence_unlock(void) { ck_pr_barrier(); return; }
# 171 "../../../include/ck_pr.h"
__attribute__((unused)) static void
ck_pr_rfo(const void *m)
{

 (void)m;
 return;
}
# 509 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_btc_int(int *target, unsigned int offset) { int previous; int punt; punt = ck_pr_md_load_int(target); previous = (int)punt; while (ck_pr_cas_int_value(target, (int)previous, (int)(previous ^ ( ((int)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_btr_int(int *target, unsigned int offset) { int previous; int punt; punt = ck_pr_md_load_int(target); previous = (int)punt; while (ck_pr_cas_int_value(target, (int)previous, (int)(previous & (~ ((int)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_bts_int(int *target, unsigned int offset) { int previous; int punt; punt = ck_pr_md_load_int(target); previous = (int)punt; while (ck_pr_cas_int_value(target, (int)previous, (int)(previous | ( ((int)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }
# 528 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_btc_uint(unsigned int *target, unsigned int offset) { unsigned int previous; unsigned int punt; punt = ck_pr_md_load_uint(target); previous = (unsigned int)punt; while (ck_pr_cas_uint_value(target, (unsigned int)previous, (unsigned int)(previous ^ ( ((unsigned int)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_btr_uint(unsigned int *target, unsigned int offset) { unsigned int previous; unsigned int punt; punt = ck_pr_md_load_uint(target); previous = (unsigned int)punt; while (ck_pr_cas_uint_value(target, (unsigned int)previous, (unsigned int)(previous & (~ ((unsigned int)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_bts_uint(unsigned int *target, unsigned int offset) { unsigned int previous; unsigned int punt; punt = ck_pr_md_load_uint(target); previous = (unsigned int)punt; while (ck_pr_cas_uint_value(target, (unsigned int)previous, (unsigned int)(previous | ( ((unsigned int)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }
# 547 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_btc_ptr(void *target, unsigned int offset) { uintptr_t previous; void * punt; punt = ck_pr_md_load_ptr(target); previous = (uintptr_t)punt; while (ck_pr_cas_ptr_value(target, (void *)previous, (void *)(previous ^ ( ((uintptr_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_btr_ptr(void *target, unsigned int offset) { uintptr_t previous; void * punt; punt = ck_pr_md_load_ptr(target); previous = (uintptr_t)punt; while (ck_pr_cas_ptr_value(target, (void *)previous, (void *)(previous & (~ ((uintptr_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_bts_ptr(void *target, unsigned int offset) { uintptr_t previous; void * punt; punt = ck_pr_md_load_ptr(target); previous = (uintptr_t)punt; while (ck_pr_cas_ptr_value(target, (void *)previous, (void *)(previous | ( ((uintptr_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }
# 566 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_btc_64(uint64_t *target, unsigned int offset) { uint64_t previous; uint64_t punt; punt = ck_pr_md_load_64(target); previous = (uint64_t)punt; while (ck_pr_cas_64_value(target, (uint64_t)previous, (uint64_t)(previous ^ ( ((uint64_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_btr_64(uint64_t *target, unsigned int offset) { uint64_t previous; uint64_t punt; punt = ck_pr_md_load_64(target); previous = (uint64_t)punt; while (ck_pr_cas_64_value(target, (uint64_t)previous, (uint64_t)(previous & (~ ((uint64_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_bts_64(uint64_t *target, unsigned int offset) { uint64_t previous; uint64_t punt; punt = ck_pr_md_load_64(target); previous = (uint64_t)punt; while (ck_pr_cas_64_value(target, (uint64_t)previous, (uint64_t)(previous | ( ((uint64_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }
# 585 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_btc_32(uint32_t *target, unsigned int offset) { uint32_t previous; uint32_t punt; punt = ck_pr_md_load_32(target); previous = (uint32_t)punt; while (ck_pr_cas_32_value(target, (uint32_t)previous, (uint32_t)(previous ^ ( ((uint32_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_btr_32(uint32_t *target, unsigned int offset) { uint32_t previous; uint32_t punt; punt = ck_pr_md_load_32(target); previous = (uint32_t)punt; while (ck_pr_cas_32_value(target, (uint32_t)previous, (uint32_t)(previous & (~ ((uint32_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_bts_32(uint32_t *target, unsigned int offset) { uint32_t previous; uint32_t punt; punt = ck_pr_md_load_32(target); previous = (uint32_t)punt; while (ck_pr_cas_32_value(target, (uint32_t)previous, (uint32_t)(previous | ( ((uint32_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }
# 604 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_btc_16(uint16_t *target, unsigned int offset) { uint16_t previous; uint16_t punt; punt = ck_pr_md_load_16(target); previous = (uint16_t)punt; while (ck_pr_cas_16_value(target, (uint16_t)previous, (uint16_t)(previous ^ ( ((uint16_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_btr_16(uint16_t *target, unsigned int offset) { uint16_t previous; uint16_t punt; punt = ck_pr_md_load_16(target); previous = (uint16_t)punt; while (ck_pr_cas_16_value(target, (uint16_t)previous, (uint16_t)(previous & (~ ((uint16_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }




__attribute__((unused)) static _Bool ck_pr_bts_16(uint16_t *target, unsigned int offset) { uint16_t previous; uint16_t punt; punt = ck_pr_md_load_16(target); previous = (uint16_t)punt; while (ck_pr_cas_16_value(target, (uint16_t)previous, (uint16_t)(previous | ( ((uint16_t)1 << offset))), &previous) == 0) ck_pr_stall(); return ((previous >> offset) & 1); }
# 668 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_inc_char_is_zero(char *target) { char previous; char punt; punt = (char)ck_pr_md_load_char(target); previous = (char)punt; while (ck_pr_cas_char_value(target, (char)previous, (char)(previous + 1), &previous) == 0) ck_pr_stall(); return previous == (char)-1; } __attribute__((unused)) static void ck_pr_inc_char_zero(char *target, _Bool *zero) { *zero = ck_pr_inc_char_is_zero(target); return; }
# 680 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_dec_char_is_zero(char *target) { char previous; char punt; punt = (char)ck_pr_md_load_char(target); previous = (char)punt; while (ck_pr_cas_char_value(target, (char)previous, (char)(previous - 1), &previous) == 0) ck_pr_stall(); return previous == (char)1; } __attribute__((unused)) static void ck_pr_dec_char_zero(char *target, _Bool *zero) { *zero = ck_pr_dec_char_is_zero(target); return; }
# 696 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_inc_int_is_zero(int *target) { int previous; int punt; punt = (int)ck_pr_md_load_int(target); previous = (int)punt; while (ck_pr_cas_int_value(target, (int)previous, (int)(previous + 1), &previous) == 0) ck_pr_stall(); return previous == (int)-1; } __attribute__((unused)) static void ck_pr_inc_int_zero(int *target, _Bool *zero) { *zero = ck_pr_inc_int_is_zero(target); return; }
# 708 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_dec_int_is_zero(int *target) { int previous; int punt; punt = (int)ck_pr_md_load_int(target); previous = (int)punt; while (ck_pr_cas_int_value(target, (int)previous, (int)(previous - 1), &previous) == 0) ck_pr_stall(); return previous == (int)1; } __attribute__((unused)) static void ck_pr_dec_int_zero(int *target, _Bool *zero) { *zero = ck_pr_dec_int_is_zero(target); return; }
# 739 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_inc_uint_is_zero(unsigned int *target) { unsigned int previous; unsigned int punt; punt = (unsigned int)ck_pr_md_load_uint(target); previous = (unsigned int)punt; while (ck_pr_cas_uint_value(target, (unsigned int)previous, (unsigned int)(previous + 1), &previous) == 0) ck_pr_stall(); return previous == (unsigned int)(2147483647 *2U +1U); } __attribute__((unused)) static void ck_pr_inc_uint_zero(unsigned int *target, _Bool *zero) { *zero = ck_pr_inc_uint_is_zero(target); return; }
# 751 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_dec_uint_is_zero(unsigned int *target) { unsigned int previous; unsigned int punt; punt = (unsigned int)ck_pr_md_load_uint(target); previous = (unsigned int)punt; while (ck_pr_cas_uint_value(target, (unsigned int)previous, (unsigned int)(previous - 1), &previous) == 0) ck_pr_stall(); return previous == (unsigned int)1; } __attribute__((unused)) static void ck_pr_dec_uint_zero(unsigned int *target, _Bool *zero) { *zero = ck_pr_dec_uint_is_zero(target); return; }
# 767 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_inc_ptr_is_zero(void *target) { uintptr_t previous; void * punt; punt = (void *)ck_pr_md_load_ptr(target); previous = (uintptr_t)punt; while (ck_pr_cas_ptr_value(target, (void *)previous, (void *)(previous + 1), &previous) == 0) ck_pr_stall(); return previous == (uintptr_t)(2147483647 *2U +1U); }
# 779 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_dec_ptr_is_zero(void *target) { uintptr_t previous; void * punt; punt = (void *)ck_pr_md_load_ptr(target); previous = (uintptr_t)punt; while (ck_pr_cas_ptr_value(target, (void *)previous, (void *)(previous - 1), &previous) == 0) ck_pr_stall(); return previous == (uintptr_t)1; }
# 795 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_inc_64_is_zero(uint64_t *target) { uint64_t previous; uint64_t punt; punt = (uint64_t)ck_pr_md_load_64(target); previous = (uint64_t)punt; while (ck_pr_cas_64_value(target, (uint64_t)previous, (uint64_t)(previous + 1), &previous) == 0) ck_pr_stall(); return previous == (uint64_t)(18446744073709551615UL); } __attribute__((unused)) static void ck_pr_inc_64_zero(uint64_t *target, _Bool *zero) { *zero = ck_pr_inc_64_is_zero(target); return; }
# 807 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_dec_64_is_zero(uint64_t *target) { uint64_t previous; uint64_t punt; punt = (uint64_t)ck_pr_md_load_64(target); previous = (uint64_t)punt; while (ck_pr_cas_64_value(target, (uint64_t)previous, (uint64_t)(previous - 1), &previous) == 0) ck_pr_stall(); return previous == (uint64_t)1; } __attribute__((unused)) static void ck_pr_dec_64_zero(uint64_t *target, _Bool *zero) { *zero = ck_pr_dec_64_is_zero(target); return; }
# 823 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_inc_32_is_zero(uint32_t *target) { uint32_t previous; uint32_t punt; punt = (uint32_t)ck_pr_md_load_32(target); previous = (uint32_t)punt; while (ck_pr_cas_32_value(target, (uint32_t)previous, (uint32_t)(previous + 1), &previous) == 0) ck_pr_stall(); return previous == (uint32_t)(4294967295U); } __attribute__((unused)) static void ck_pr_inc_32_zero(uint32_t *target, _Bool *zero) { *zero = ck_pr_inc_32_is_zero(target); return; }
# 835 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_dec_32_is_zero(uint32_t *target) { uint32_t previous; uint32_t punt; punt = (uint32_t)ck_pr_md_load_32(target); previous = (uint32_t)punt; while (ck_pr_cas_32_value(target, (uint32_t)previous, (uint32_t)(previous - 1), &previous) == 0) ck_pr_stall(); return previous == (uint32_t)1; } __attribute__((unused)) static void ck_pr_dec_32_zero(uint32_t *target, _Bool *zero) { *zero = ck_pr_dec_32_is_zero(target); return; }
# 851 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_inc_16_is_zero(uint16_t *target) { uint16_t previous; uint16_t punt; punt = (uint16_t)ck_pr_md_load_16(target); previous = (uint16_t)punt; while (ck_pr_cas_16_value(target, (uint16_t)previous, (uint16_t)(previous + 1), &previous) == 0) ck_pr_stall(); return previous == (uint16_t)(65535); } __attribute__((unused)) static void ck_pr_inc_16_zero(uint16_t *target, _Bool *zero) { *zero = ck_pr_inc_16_is_zero(target); return; }
# 863 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_dec_16_is_zero(uint16_t *target) { uint16_t previous; uint16_t punt; punt = (uint16_t)ck_pr_md_load_16(target); previous = (uint16_t)punt; while (ck_pr_cas_16_value(target, (uint16_t)previous, (uint16_t)(previous - 1), &previous) == 0) ck_pr_stall(); return previous == (uint16_t)1; } __attribute__((unused)) static void ck_pr_dec_16_zero(uint16_t *target, _Bool *zero) { *zero = ck_pr_dec_16_is_zero(target); return; }
# 879 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_inc_8_is_zero(uint8_t *target) { uint8_t previous; uint8_t punt; punt = (uint8_t)ck_pr_md_load_8(target); previous = (uint8_t)punt; while (ck_pr_cas_8_value(target, (uint8_t)previous, (uint8_t)(previous + 1), &previous) == 0) ck_pr_stall(); return previous == (uint8_t)(255); } __attribute__((unused)) static void ck_pr_inc_8_zero(uint8_t *target, _Bool *zero) { *zero = ck_pr_inc_8_is_zero(target); return; }
# 891 "../../../include/ck_pr.h"
__attribute__((unused)) static _Bool ck_pr_dec_8_is_zero(uint8_t *target) { uint8_t previous; uint8_t punt; punt = (uint8_t)ck_pr_md_load_8(target); previous = (uint8_t)punt; while (ck_pr_cas_8_value(target, (uint8_t)previous, (uint8_t)(previous - 1), &previous) == 0) ck_pr_stall(); return previous == (uint8_t)1; } __attribute__((unused)) static void ck_pr_dec_8_zero(uint8_t *target, _Bool *zero) { *zero = ck_pr_dec_8_is_zero(target); return; }
# 945 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_not_char(char *target) { char previous; char punt; punt = (char)ck_pr_md_load_char(target); previous = (char)punt; while (ck_pr_cas_char_value(target, (char)previous, (char)(~ previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_char(char *target) { char previous; char punt; punt = (char)ck_pr_md_load_char(target); previous = (char)punt; while (ck_pr_cas_char_value(target, (char)previous, (char)(- previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_char_zero(char *target, _Bool *zero) { char previous; char punt; punt = (char)ck_pr_md_load_char(target); previous = (char)punt; while (ck_pr_cas_char_value(target, (char)previous, (char)(-previous), &previous) == 0) ck_pr_stall(); *zero = previous == 0; return; }
# 964 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_not_int(int *target) { int previous; int punt; punt = (int)ck_pr_md_load_int(target); previous = (int)punt; while (ck_pr_cas_int_value(target, (int)previous, (int)(~ previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_int(int *target) { int previous; int punt; punt = (int)ck_pr_md_load_int(target); previous = (int)punt; while (ck_pr_cas_int_value(target, (int)previous, (int)(- previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_int_zero(int *target, _Bool *zero) { int previous; int punt; punt = (int)ck_pr_md_load_int(target); previous = (int)punt; while (ck_pr_cas_int_value(target, (int)previous, (int)(-previous), &previous) == 0) ck_pr_stall(); *zero = previous == 0; return; }
# 993 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_not_uint(unsigned int *target) { unsigned int previous; unsigned int punt; punt = (unsigned int)ck_pr_md_load_uint(target); previous = (unsigned int)punt; while (ck_pr_cas_uint_value(target, (unsigned int)previous, (unsigned int)(~ previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_uint(unsigned int *target) { unsigned int previous; unsigned int punt; punt = (unsigned int)ck_pr_md_load_uint(target); previous = (unsigned int)punt; while (ck_pr_cas_uint_value(target, (unsigned int)previous, (unsigned int)(- previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_uint_zero(unsigned int *target, _Bool *zero) { unsigned int previous; unsigned int punt; punt = (unsigned int)ck_pr_md_load_uint(target); previous = (unsigned int)punt; while (ck_pr_cas_uint_value(target, (unsigned int)previous, (unsigned int)(-previous), &previous) == 0) ck_pr_stall(); *zero = previous == 0; return; }
# 1012 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_not_ptr(void *target) { uintptr_t previous; void * punt; punt = (void *)ck_pr_md_load_ptr(target); previous = (uintptr_t)punt; while (ck_pr_cas_ptr_value(target, (void *)previous, (void *)(~ previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_ptr(void *target) { uintptr_t previous; void * punt; punt = (void *)ck_pr_md_load_ptr(target); previous = (uintptr_t)punt; while (ck_pr_cas_ptr_value(target, (void *)previous, (void *)(- previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_ptr_zero(void *target, _Bool *zero) { uintptr_t previous; void * punt; punt = (void *)ck_pr_md_load_ptr(target); previous = (uintptr_t)punt; while (ck_pr_cas_ptr_value(target, (void *)previous, (void *)(-previous), &previous) == 0) ck_pr_stall(); *zero = previous == 0; return; }
# 1031 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_not_64(uint64_t *target) { uint64_t previous; uint64_t punt; punt = (uint64_t)ck_pr_md_load_64(target); previous = (uint64_t)punt; while (ck_pr_cas_64_value(target, (uint64_t)previous, (uint64_t)(~ previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_64(uint64_t *target) { uint64_t previous; uint64_t punt; punt = (uint64_t)ck_pr_md_load_64(target); previous = (uint64_t)punt; while (ck_pr_cas_64_value(target, (uint64_t)previous, (uint64_t)(- previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_64_zero(uint64_t *target, _Bool *zero) { uint64_t previous; uint64_t punt; punt = (uint64_t)ck_pr_md_load_64(target); previous = (uint64_t)punt; while (ck_pr_cas_64_value(target, (uint64_t)previous, (uint64_t)(-previous), &previous) == 0) ck_pr_stall(); *zero = previous == 0; return; }
# 1050 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_not_32(uint32_t *target) { uint32_t previous; uint32_t punt; punt = (uint32_t)ck_pr_md_load_32(target); previous = (uint32_t)punt; while (ck_pr_cas_32_value(target, (uint32_t)previous, (uint32_t)(~ previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_32(uint32_t *target) { uint32_t previous; uint32_t punt; punt = (uint32_t)ck_pr_md_load_32(target); previous = (uint32_t)punt; while (ck_pr_cas_32_value(target, (uint32_t)previous, (uint32_t)(- previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_32_zero(uint32_t *target, _Bool *zero) { uint32_t previous; uint32_t punt; punt = (uint32_t)ck_pr_md_load_32(target); previous = (uint32_t)punt; while (ck_pr_cas_32_value(target, (uint32_t)previous, (uint32_t)(-previous), &previous) == 0) ck_pr_stall(); *zero = previous == 0; return; }
# 1069 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_not_16(uint16_t *target) { uint16_t previous; uint16_t punt; punt = (uint16_t)ck_pr_md_load_16(target); previous = (uint16_t)punt; while (ck_pr_cas_16_value(target, (uint16_t)previous, (uint16_t)(~ previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_16(uint16_t *target) { uint16_t previous; uint16_t punt; punt = (uint16_t)ck_pr_md_load_16(target); previous = (uint16_t)punt; while (ck_pr_cas_16_value(target, (uint16_t)previous, (uint16_t)(- previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_16_zero(uint16_t *target, _Bool *zero) { uint16_t previous; uint16_t punt; punt = (uint16_t)ck_pr_md_load_16(target); previous = (uint16_t)punt; while (ck_pr_cas_16_value(target, (uint16_t)previous, (uint16_t)(-previous), &previous) == 0) ck_pr_stall(); *zero = previous == 0; return; }
# 1088 "../../../include/ck_pr.h"
__attribute__((unused)) static void ck_pr_not_8(uint8_t *target) { uint8_t previous; uint8_t punt; punt = (uint8_t)ck_pr_md_load_8(target); previous = (uint8_t)punt; while (ck_pr_cas_8_value(target, (uint8_t)previous, (uint8_t)(~ previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_8(uint8_t *target) { uint8_t previous; uint8_t punt; punt = (uint8_t)ck_pr_md_load_8(target); previous = (uint8_t)punt; while (ck_pr_cas_8_value(target, (uint8_t)previous, (uint8_t)(- previous), &previous) == 0) ck_pr_stall(); return; }




__attribute__((unused)) static void ck_pr_neg_8_zero(uint8_t *target, _Bool *zero) { uint8_t previous; uint8_t punt; punt = (uint8_t)ck_pr_md_load_8(target); previous = (uint8_t)punt; while (ck_pr_cas_8_value(target, (uint8_t)previous, (uint8_t)(-previous), &previous) == 0) ck_pr_stall(); *zero = previous == 0; return; }
# 1152 "../../../include/ck_pr.h"
__attribute__((unused)) static char ck_pr_fas_char(char *target, char update) { char previous; previous = ck_pr_md_load_char(target); while (ck_pr_cas_char_value(target, previous, update, &previous) == 0) ck_pr_stall(); return (previous); }
# 1166 "../../../include/ck_pr.h"
__attribute__((unused)) static int ck_pr_fas_int(int *target, int update) { int previous; previous = ck_pr_md_load_int(target); while (ck_pr_cas_int_value(target, previous, update, &previous) == 0) ck_pr_stall(); return (previous); }
# 1195 "../../../include/ck_pr.h"
__attribute__((unused)) static unsigned int ck_pr_fas_uint(unsigned int *target, unsigned int update) { unsigned int previous; previous = ck_pr_md_load_uint(target); while (ck_pr_cas_uint_value(target, previous, update, &previous) == 0) ck_pr_stall(); return (previous); }
# 1209 "../../../include/ck_pr.h"
__attribute__((unused)) static void * ck_pr_fas_ptr(void *target, void * update) { void * previous; previous = ck_pr_md_load_ptr(target); while (ck_pr_cas_ptr_value(target, previous, update, &previous) == 0) ck_pr_stall(); return (previous); }
# 1223 "../../../include/ck_pr.h"
__attribute__((unused)) static uint64_t ck_pr_fas_64(uint64_t *target, uint64_t update) { uint64_t previous; previous = ck_pr_md_load_64(target); while (ck_pr_cas_64_value(target, previous, update, &previous) == 0) ck_pr_stall(); return (previous); }
# 1237 "../../../include/ck_pr.h"
__attribute__((unused)) static uint32_t ck_pr_fas_32(uint32_t *target, uint32_t update) { uint32_t previous; previous = ck_pr_md_load_32(target); while (ck_pr_cas_32_value(target, previous, update, &previous) == 0) ck_pr_stall(); return (previous); }
# 1251 "../../../include/ck_pr.h"
__attribute__((unused)) static uint16_t ck_pr_fas_16(uint16_t *target, uint16_t update) { uint16_t previous; previous = ck_pr_md_load_16(target); while (ck_pr_cas_16_value(target, previous, update, &previous) == 0) ck_pr_stall(); return (previous); }
# 1265 "../../../include/ck_pr.h"
__attribute__((unused)) static uint8_t ck_pr_fas_8(uint8_t *target, uint8_t update) { uint8_t previous; previous = ck_pr_md_load_8(target); while (ck_pr_cas_8_value(target, previous, update, &previous) == 0) ck_pr_stall(); return (previous); }
# 32 "../../../include/ck_sequence.h" 2
# 1 "../../../include/ck_stdbool.h" 1
# 33 "../../../include/ck_sequence.h" 2

struct ck_sequence {
 unsigned int sequence;
};
typedef struct ck_sequence ck_sequence_t;



__attribute__((unused)) static void
ck_sequence_init(struct ck_sequence *sq)
{
 ck_pr_md_store_uint(&sq->sequence, 0);
 return;
}

__attribute__((unused)) static unsigned int
ck_sequence_read_begin(const struct ck_sequence *sq)
{
 unsigned int version;

 for (;;) {
  version = ck_pr_md_load_uint(((&sq->sequence)));
  if ((__builtin_expect(!!((version & 1) == 0), 1)))
   break;
  ck_pr_stall();
 }

 ck_pr_fence_load();
 return version;
}

__attribute__((unused)) static _Bool
ck_sequence_read_retry(const struct ck_sequence *sq, unsigned int version)
{





 ck_pr_fence_load();
 return ck_pr_md_load_uint(((&sq->sequence))) != version;
}
# 96 "../../../include/ck_sequence.h"
__attribute__((unused)) static void
ck_sequence_write_begin(struct ck_sequence *sq)
{
 ck_pr_md_store_uint(((&sq->sequence)), ((sq->sequence + 1)));
 ck_pr_fence_store();
 return;
}




__attribute__((unused)) static void
ck_sequence_write_end(struct ck_sequence *sq)
{
 ck_pr_fence_store();
 ck_pr_md_store_uint(((&sq->sequence)), ((sq->sequence + 1)));
 return;
}
# 6 "cks_example.c" 2

static struct example {
        int x;
        int y;
} global;

static ck_sequence_t seqlock = { .sequence = 0 };



void *reader(void *arg)
{
        struct example copy;
        unsigned int version;

        for (int i=0;i<2 ;i++) {
         do {
                 version = ck_sequence_read_begin(&seqlock);
                 copy.x = global.x;
                 copy.y = global.y;
         } while (ck_sequence_read_retry(&seqlock, version));
  assert ((copy.x==copy.y) && "copy.x==copy.y");
 }
# 37 "cks_example.c"
        return ((void*)0);
}

void *writer(void *arg)
{

        for (int i=0;i<2 ;i++) {
                ck_sequence_write_begin(&seqlock);
                global.x++;
                global.y = global.x;
                ck_sequence_write_end(&seqlock);
        }

        return ((void*)0);
}

int main()
{
 pthread_t treader, twriter;

 pthread_create(&treader,((void*)0), reader, ((void*)0));
 pthread_create(&twriter,((void*)0), writer, ((void*)0));

 pthread_join(treader,((void*)0));
 pthread_join(twriter,((void*)0));


 return 0;
}

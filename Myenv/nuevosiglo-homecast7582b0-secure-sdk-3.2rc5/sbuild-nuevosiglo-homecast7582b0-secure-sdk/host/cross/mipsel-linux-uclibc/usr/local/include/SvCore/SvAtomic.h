/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_ATOMIC_H_
#define SV_ATOMIC_H_

/**
 * @file SvAtomic.h Atomic operations
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvAtomic Atomic operations
 * @ingroup SvCore
 * @{
 *
 * Primitives for performing atomic operations on in-memory variables.
 **/


#if defined __mips__

# if defined __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   // use gcc intrinsic
   return __sync_val_compare_and_swap(p, x, y);
}

# else

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   long int tmp, result;
   __asm__ __volatile__ (
   "\t"     ".set    mips3"      "\n"
   "\t"     ".set    noreorder"  "\n"
   "\t"     "sync"               "\n"
   "1:\t"   "ll      %1, %0"     "\n"
   "\t"     "bne     %1, %3, 2f" "\n"
   "\t"     "add     %2, $0, %4" "\n"
   "\t"     "sc      %2, %0"     "\n"
   "\t"     "beqz    %2, 1b"     "\n"
   "\t"     "nop"                "\n"
   "2:\t"   "sync"               "\n"
   "\t"     ".set    reorder"    "\n"
   "\t"     ".set    mips0"      "\n"
   : "=m" (*p), "=&r" (result), "=&r" (tmp)
   : "Ir" (x), "Ir" (y), "m" (*p)
   : "memory"
   );
   return result;
}

# endif

#define SvAtomicIncrementDefined
static inline long int
SvAtomicIncrement(volatile long int *p)
{
   long int tmp, result;
   __asm__ __volatile__ (
   "\t"     ".set    mips3"      "\n"
   "\t"     ".set    noreorder"  "\n"
   "\t"     "sync"               "\n"
   "1:\t"   "ll      %1, %0"     "\n"
   "\t"     "addiu   %2, %1, 1"  "\n"
   "\t"     "sc      %2, %0"     "\n"
   "\t"     "beqz    %2, 1b"     "\n"
   "\t"     "nop"                "\n"
   "2:\t"   "sync"               "\n"
   "\t"     ".set    reorder"    "\n"
   "\t"     ".set    mips0"      "\n"
   : "=m" (*p), "=&r" (result), "=&r" (tmp)
   : "m" (*p)
   : "memory"
   );
   return result;
}

#define SvAtomicDecrementDefined
static inline long int
SvAtomicDecrement(volatile long int *p)
{
   long int tmp, result;
   __asm__ __volatile__ (
   "\t"     ".set    mips3"      "\n"
   "\t"     ".set    noreorder"  "\n"
   "\t"     "sync"               "\n"
   "1:\t"   "ll      %1, %0"     "\n"
   "\t"     "addiu   %2, %1, -1" "\n"
   "\t"     "sc      %2, %0"     "\n"
   "\t"     "beqz    %2, 1b"     "\n"
   "\t"     "nop"                "\n"
   "2:\t"   "sync"               "\n"
   "\t"     ".set    reorder"    "\n"
   "\t"     ".set    mips0"      "\n"
   : "=m" (*p), "=&r" (tmp), "=&r" (result)
   : "m" (*p)
   : "memory"
   );
   return result;
}

#elif defined __i386__

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   long int result;
   __asm__ __volatile__ (
   "\t"     "lock; cmpxchgl %3, %1"  "\n"
   : "=a" (result), "=m" (*p)
   : "0" (x), "r" (y)
   : "cc"
   );
   return result;
}

#define SvAtomicIncrementDefined
static inline long int
SvAtomicIncrement(volatile long int *p)
{
   long int value = 1L;
   __asm__ __volatile__ (
   "\t"    "lock; xaddl %0, %1"    "\n"
   : "=r" (value), "=m" (*p)
   : "0" (value), "m" (*p)
   : "cc"
   );
   return value;
}

#define SvAtomicDecrementDefined
static inline long int
SvAtomicDecrement(volatile long int *p)
{
   long int value = -1L;
   __asm__ __volatile__ (
   "\t"    "lock; xaddl %0, %1"    "\n"
   : "=r" (value), "=m" (*p)
   : "0" (value), "m" (*p)
   : "cc"
   );
   return value - 1;
}

#elif defined __x86_64__

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   long int result;
   __asm__ __volatile__ (
   "\t"     "lock; cmpxchgq %3, %1"  "\n"
   : "=a" (result), "=m" (*p)
   : "0" (x), "r" (y)
   : "cc"
   );
   return result;
}

#define SvAtomicIncrementDefined
static inline long int
SvAtomicIncrement(volatile long int *p)
{
   long int value = 1L;
   __asm__ __volatile__ (
   "\t"    "lock; xaddq %0, %1"    "\n"
   : "=r" (value), "=m" (*p)
   : "0" (value), "m" (*p)
   : "cc"
   );
   return value;
}

#define SvAtomicDecrementDefined
static inline long int
SvAtomicDecrement(volatile long int *p)
{
   long int value = -1L;
   __asm__ __volatile__ (
   "\t"    "lock; xaddq %0, %1"    "\n"
   : "=r" (value), "=m" (*p)
   : "0" (value), "m" (*p)
   : "cc"
   );
   return value - 1;
}

#elif defined __ppc__

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   long int result;
   __asm__ __volatile__ (
   "\t"     "lwsync"             "\n"
   "1:\t"   "lwarx   %1, 0, %2"  "\n"
   "\t"     "cmpw    0, %1, %3"  "\n"
   "\t"     "bne-    2f"         "\n"
   "\t"     "stwcx.  %4, 0, %2"  "\n"
   "\t"     "bne-    1b"         "\n"
   "\t"     "sync"               "\n"
   "2:\t"
   : "=m" (*p), "=&r" (result)
   : "r" (p), "r" (x), "r" (y), "m" (*p)
   : "cc", "memory"
   );
   return result;
}

#elif defined __ppc64__

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   long int result;
   __asm__ __volatile__ (
   "\t"     "lwsync"             "\n"
   "1:\t"   "ldarx   %1, 0, %2"  "\n"
   "\t"     "cmpd    0, %1, %3"  "\n"
   "\t"     "bne-    2f"         "\n"
   "\t"     "stdcx.  %4, 0, %2"  "\n"
   "\t"     "bne-    1b"         "\n"
   "\t"     "sync"               "\n"
   "2:\t"
   : "=m" (*p), "=&r" (result)
   : "r" (p), "r" (x), "r" (y), "m" (*p)
   : "cc", "memory"
   );
   return result;
}

#elif defined __arm__

# if defined __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   // use gcc intrinsic
   return __sync_val_compare_and_swap(p, x, y);
}

# elif (   defined __ARM_ARCH_2__ || defined __ARM_ARCH_3__ \
        || defined __ARM_ARCH_3M__ || defined __ARM_ARCH_4__ \
        || defined __ARM_ARCH_4T__ || defined __ARM_ARCH_5__ \
        || defined __ARM_ARCH_5T__)

// atomic operations are more complex on ARM, because it supports
// them natively since 6th generation cores only; for older chips
// we have to use special kernel helpers

typedef long int (__kernel_cmpxchg_t)(long int x, long int y, volatile long int *p);
#define __kernel_cmpxchg (*(__kernel_cmpxchg_t *) 0xffff0fc0)

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   long int val;
   do {
      val = *p;
      if (val != x)
         return val;
   } while (__kernel_cmpxchg(val, y, p) != 0);
   return x;
}

# else

static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   long int result, val;
   __asm__ __volatile__ (
   "1:\t"   "ldrex   %1, [%2]"      "\n"
   "\t"     "mov     %0, #0"        "\n"
   "\t"     "teq     %1, %3"        "\n"
   "\t"     "strexeq %0, %4, [%2]"  "\n"
   "\t"     "teq     %0, #0"        "\n"
   "\t"     "bne     1b"            "\n"
   : "=&r" (val), "=&r" (result)
   : "r" (p), "Ir" (x), "r" (y)
   : "cc"
   );
   return result;
}

# endif

#elif defined __sh__

/*
 * SH-4 does not really have compare-and-swap. To overcome this deficiency,
 * Linux kernel implements so-called "gUSA" ("g" User Space Atomicity) protocol
 * (see http://lc.linux.or.jp/lc2002/papers/niibe0919h.pdf). It guarantees
 * that specially marked sequence of instructions (r15 >= 0xc0000000)
 * will be executed atomically (more strictly speaking: will be restarted
 * if it is interrupted).
 */
static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   long int result;
   __asm__ __volatile__ (
            ".align 2"           "\n"
   // put address of the first instruction after atomic sequence in r0
   "\t"     "mova    1f, r0"     "\n"
   "\t"     "nop"                "\n"
   // save stack pointer (r15) in r1
   "\t"     "mov     r15, r1"    "\n"
   // put offset of the first instruction of the atomic sequence
   // relative to r0 in r15 (stack pointer)
   "\t"     "mov     #-8, r15"   "\n"
   // here starts our atomic sequence of instructions
   "0:\t"   "mov.l   @%1, %0"    "\n"
   "\t"     "cmp/eq  %0, %2"     "\n"
   "\t"     "bf      1f"         "\n"
   "\t"     "mov.l   %3, @%1"    "\n"
   // here ends the atomic sequence, restore stack pointer
   "1:\t"   "mov r1, r15"
   : "=&r" (result)
   : "r" (p), "r" (x), "r" (y)
   : "r0", "r1", "t", "memory"
   );
   return result;
}

#define SvAtomicIncrementDefined
static inline long int
SvAtomicIncrement(volatile long int *p)
{
   long int value, addend = 1L;
   __asm__ __volatile__ (
            ".align 2"           "\n"
   "\t"     "mova    1f, r0"     "\n"
   "\t"     "nop"                "\n"
   "\t"     "mov     r15, r1"    "\n"
   "\t"     "mov     #-8, r15"   "\n"
   "0:\t"   "mov.l   @%2, %0"    "\n"
   "\t"     "mov     #1, %1"     "\n"
   "\t"     "add     %0, %1"     "\n"
   "\t"     "mov.l   %1, @%2"    "\n"
   "1:\t"   "mov r1, r15"
   : "=&r" (value)
   : "r" (addend), "r" (p)
   : "r0", "r1", "t", "memory"
   );
   return value;
}

#define SvAtomicDecrementDefined
static inline long int
SvAtomicDecrement(volatile long int *p)
{
   long int value;
   __asm__ __volatile__ (
            ".align 2"           "\n"
   "\t"     "mova    1f, r0"     "\n"
   "\t"     "mov     r15, r1"    "\n"
   "\t"     "mov     #-6, r15"   "\n"
   "0:\t"   "mov.l   @%1, %0"    "\n"
   "\t"     "add     #-1, %0"    "\n"
   "\t"     "mov.l   %0, @%1"    "\n"
   "1:\t"   "mov r1, r15"
   : "=&r" (value)
   : "r" (p)
   : "r0", "r1", "t", "memory"
   );
   return value;
}

#elif (defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) && (__SIZEOF_LONG__ == 4)) || defined DOXYGEN

/**
 * Atomic compare-and-swap primitive.
 *
 * This function atomically sets the variable pointed to by @a p
 * to @a y if and only if its previous value was equal to @a x.
 * Function returns the value of a variable pointed by @a p
 * before operation: if it is equal to @a x the operation was
 * successful, otherwise it failed.
 *
 * This function can be used as a base for implementing various
 * operations like atomic increments and decrements.
 *
 * The type of all variables is long int, because it can be used
 * to atomically swap pointers on both 32 bit and 64 bit platforms,
 * and its size is equal to int (so there is no performance penalty)
 * on most common 32 bit platforms.
 *
 * @param[in] p pointer to a variable that will be changed
 * @param[in] x value to compare @c *p with
 * @param[in] y new value to set @c *p to
 * @return value pointed by @a p before operation
 **/
static inline long int
SvCAS(volatile long int *p, long int x, long int y)
{
   // use gcc intrinsic
   return __sync_val_compare_and_swap(p, x, y);
}

#else

# error "SvCAS() for this platform is not defined!"

#endif


#ifndef SvAtomicIncrementDefined
/**
 * Atomic increment primitive.
 *
 * @param[in] p pointer to a variable that is to be incremented
 * @return value before incrementing
 **/
static inline long int
SvAtomicIncrement(volatile long int *p)
{
   long int prev;
   do {
      prev = *p;
   } while (SvCAS(p, prev, prev + 1) != prev);
   return prev;
}
#endif


#ifndef SvAtomicDecrementDefined
/**
 * Atomic decrement primitive.
 *
 * @param[in] p pointer to a variable that is to be decremented
 * @return value after decrementing
 **/
static inline long int
SvAtomicDecrement(volatile long int *p)
{
   long int prev;
   do {
      prev = *p;
   } while (SvCAS(p, prev, prev - 1) != prev);
   return prev - 1;
}
#endif


/**
 * Return @c true when evaluated for the first time, then always @c false.
 * @hideinitializer
 **/
#define SvOnce()\
({\
    static volatile long int done = 0;\
    bool first = !SvCAS(&done, 0, 1);\
    first;\
})

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

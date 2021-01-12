/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_THREAD_LOCAL_STORAGE_H_
#define SV_THREAD_LOCAL_STORAGE_H_

#include <SvCore/SvCommonDefs.h>

#if defined __linux__
# include <features.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (defined __GLIBC__ && !defined __UCLIBC__) || defined __sun__
# define SvTLSPtr __thread void *
# define SvTLSPtrInitializer ((void *) 0)

static inline void *SvTLSPtrGet(void **var)
{
    return *var;
}

static inline void SvTLSPtrSet(void **var, void *value)
{
    *var = value;
}

#else // (__GLIBC__ && !__UCLIBC__) || __sun__

# include <pthread.h>
typedef struct {
    /// @c 0 == uninitialized, @c 1 == initialized, @c -1 == locked
    volatile long int once;
    pthread_key_t key;
} SvTLSPtr;

// I'm not proud from this solution, but g++ in version 4.2 doesn't allow initialize
// struct in C style, so we have to use that fix.
#ifdef __cplusplus
    #define SvTLSPtrInitializer { 0 }
#else
    #define SvTLSPtrInitializer { .once = 0 }
#endif

extern void
SvTLSPtrInit(SvTLSPtr *var);

static inline void *SvTLSPtrGet(SvTLSPtr *var)
{
    if (unlikely(var->once <= 0L))
        SvTLSPtrInit(var);
    return pthread_getspecific(var->key);
}

static inline void SvTLSPtrSet(SvTLSPtr *var, void *value)
{
    if (unlikely(var->once <= 0L))
        SvTLSPtrInit(var);
    pthread_setspecific(var->key, value);
}

#endif // __GLIBC__

#ifdef __cplusplus
}
#endif

#endif // SV_THREAD_LOCAL_STORAGE_H_

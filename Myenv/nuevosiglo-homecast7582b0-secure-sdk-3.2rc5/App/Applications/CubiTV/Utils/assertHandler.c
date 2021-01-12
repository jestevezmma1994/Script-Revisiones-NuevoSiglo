/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include <SvCore/SvLog.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>


#ifndef NDEBUG

// uClibc defines both __GLIBC__ and __UCLIBC__
# if defined __GLIBC__

#  if defined __UCLIBC__
#    if (__UCLIBC_MAJOR__ >= 0) && (__UCLIBC_MINOR__ >= 9) && (__UCLIBC_SUBLEVEL__ >= 32)
extern void __assert __P((const char *__assertion, const char *__file,
               unsigned int __line, const char *__function))
#    else // __UCLIBC_MAJOR__
extern void __assert __P((const char *__assertion, const char *__file,
               int __line, const char *__function))
#    endif // __UCLIBC_MAJOR__
#  else // __UCLIBC__
void __assert_fail (__const char *__assertion, __const char *__file,
               unsigned int __line, __const char *__function)
#  endif // __UCLIBC__
{
    SvLogError("%s:%i [%s] assert(%s) failed", __file, __line, __function, __assertion);
    fflush(stdout);
    fflush(stderr);
#   if !defined __i386__ && !defined __x86_64__
    *((int *) NULL + 3) = 0xdeadbeef;
#   endif
    abort();
}

# endif // __GLIBC__

# if !defined __i386__ && !defined __x86_64__

void abort(void)
{
    *((int *) NULL + 9) = 0xbeefdead;
    kill(getpid(), SIGABRT);
    exit(1);
}

# endif

#endif // NDEBUG

/* This file does not belong to the "fibers" library.
 * It is a link to external toolchain and may be freely changed or removed.
 */

#ifndef FIBERS_EXTERNAL_SYSTEM_DEFS_H
#define FIBERS_EXTERNAL_SYSTEM_DEFS_H

#include <ctype.h>
#include <stdlib.h>

#define INL inline

#define myassert(expr) for (;;) { \
    if (!(expr)) { \
        abort(); \
    }; \
    break; \
  };

#if defined (_MSC_VER)
#  define FRIEND(FULLNAME, SHORTNAME) friend SHORTNAME
#elif defined (__clang__)
#  define FRIEND(FULLNAME, SHORTNAME) friend SHORTNAME
#else
#  define FRIEND(FULLNAME, SHORTNAME) friend class FULLNAME
#endif

#endif // #ifdef FIBERS_EXTERNAL_SYSTEM_DEFS_H

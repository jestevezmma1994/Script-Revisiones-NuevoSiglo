/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_MEM_STATS_H_
#define SV_MEM_STATS_H_

/**
 * @file SvMemStats.h
 * @brief Utility for retrieving memory usage statistics
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <unistd.h> // for ssize_t

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvMemStats Retrieving memory usage statistics
 * @ingroup SvQuirks
 * @{
 **/

struct SvMallocStats {
    /// total size of memory chunks in use, in bytes (@c -1 if unknown)
    ssize_t usedSize;
    /**
     * total size of memory chunks allocated from OS but currently unused,
     * in bytes (@c -1 if unknown)
     **/
    ssize_t freeSize;
    /// number of chunks in @a freeTotal, @c -1 if unknown
    ssize_t freeCount;
    /**
     * total size of memory allocated on heap (using sbrk() system call),
     * in bytes (@c -1 if unknown)
     **/
    ssize_t heapSize;
    /// total size of memory allocated with mmap(), it bytes (@c -1 if unknown)
    ssize_t mmapSize;
    /// number of chunks allocated with mmap(), @c -1 if unknown
    ssize_t mmapCount;
};


/**
 * Get memory usage statistics.
 *
 * @param[out] stats     current memory usage statistics
 **/
extern void
SvGetMallocStats(struct SvMallocStats *stats);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

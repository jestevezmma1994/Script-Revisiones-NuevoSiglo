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

#ifndef SV_CACHE_FLUSH_H_
#define SV_CACHE_FLUSH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file SvCacheFlush.h
 * @brief Utility for flushing memory cache
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>

/**
 * @defgroup SvCacheFlush Flushing memory caches
 * @ingroup SvQuirks
 * @{
 **/

/**
 * Flush memory caches.
 *
 * This function is used to flush memory cache before reading/writing
 * data using DMA techniques (for example: reading/writing mmap()ed data
 * to/from a file opened with O_DIRECT option on linux).
 *
 * @param[in] ptr       address of a memory block to flush
 * @param[in] len       size of the block at @a ptr
 **/
extern void
SvCacheFlush(const void *ptr, size_t len);


/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

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

#ifndef QB_MEM_POOL_H_
#define QB_MEM_POOL_H_

/**
 * @file QBMemPool.h
 * @brief Memory pool interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBMemPool Memory pool interface
 * @ingroup QBSHMem
 * @{
 *
 * @image html QBMemPool.png
 **/

/**
 * Get runtime type identification object representing
 * memory pool interface.
 *
 * @return memory pool interface
 **/
extern SvInterface
QBMemPool_getInterface(void);


/**
 * QBMemPool interface.
 **/
typedef const struct QBMemPool_ {
    /**
     * Get number of segments allocated from the memory pool.
     *
     * @param[in] self_     a handle to an object implementing QBMemPool
     * @return              number of allocated segments or -1 in case of error
     **/
    int (*getSegmentsCount)(SvObject self_);

    /**
     * Get total length of all segments allocated from the memory pool.
     *
     * @param[in] self_     a handle to an object implementing QBMemPool
     * @return              total allocated memory (in bytes),
     *                      (size_t) -1 in case of error
     **/
    size_t (*getTotalLength)(SvObject self_);

    /**
     * Allocate new segment of requested size.
     *
     * @param[in] self      a handle to an object implementing QBMemPool
     * @param[in] length    length of the segment, in bytes (it can be rounded
     *                      up to a multiple of page size)
     * @param[in] rights    UNIX access rights to the segment
     * @param[out] errorOut error info
     * @return              newly allocated segment or @c NULL in case of error
     **/
    SvObject (*allocateSegment)(SvObject self_,
                                size_t length,
                                unsigned int rights,
                                SvErrorInfo *errorOut);

    /**
     * Find segment containing given @a @a address.
     *
     * This method returns a handle to a segment containing given
     * @a address. Caller is responsible for releasing returned object.
     *
     * @note Memory pool can keep track of allocated segments using
     * data structures that do not provide fast lookups by segment address.
     * When memory pool has a lot of allocated segments, lookups can
     * take significant amount of time.
     *
     * @param[in] self_     a handle to an object implementing QBMemPool
     * @param[in] address   memory address
     * @return              segment containing @a address, @c NULL if not found
     **/
    SvObject (*findSegmentByAddr)(SvObject self_,
                                  void *address);
} *QBMemPool;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

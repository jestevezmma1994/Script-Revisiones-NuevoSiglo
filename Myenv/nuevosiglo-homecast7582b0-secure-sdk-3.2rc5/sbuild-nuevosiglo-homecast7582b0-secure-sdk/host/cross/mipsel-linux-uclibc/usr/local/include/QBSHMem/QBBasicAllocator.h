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

#ifndef QB_BASIC_ALLOCATOR_H_
#define QB_BASIC_ALLOCATOR_H_

/**
 * @file QBBasicAllocator.h
 * @brief Basic memory allocator class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBBasicAllocator Basic memory allocator class
 * @ingroup QBSHMem
 * @{
 *
 * QBBasicAllocator is a simple memory allocator implementation that uses
 * memory pool implementing @ref QBMemPool as underlying allocation mechanism.
 *
 * @image html QBBasicAllocator.png
 **/

/**
 * Basic memory allocator class.
 * @class QBBasicAllocator
 * @extends SvObject
 **/
typedef struct QBBasicAllocator_ *QBBasicAllocator;

/**
 * Basic memory allocator statistics.
 **/
typedef struct {
    /// number of allocated blocks
    size_t totalBlocksCount;
    /// number of bytes used by allocated blocks
    size_t totalUsed;
    /// number of bytes available without allocating new segments
    size_t totalFree;

    /// number of allocated big blocks
    size_t bigBlocksCount;
    /// number of bytes used by big blocks
    size_t bigBlocksUsed;

    /// number of segments allocated from memory pool
    size_t segmentsCount;
    /// total size of all allocated segments;
    size_t segmentsSize;

    /// size of the largest block available without allocating new segments
    size_t largestAvailableBlockSize;

    /// reserved for future use
    size_t reserved[3];
} QBBasicAllocatorStats;

/**
 * Single item on the list of allocated blocks.
 **/
typedef struct {
    /// block header as seen by the allocator
    void *blockHeader;
    /// pointer to the block's data as returned by allocator
    void *ptr;
    /// size of block's data in bytes
    size_t size;
    /// requested block alignment
    size_t alignment;
} QBBasicAllocatorBlocksListItem;

typedef struct {
    /// Min value of the big block size threshold.
    size_t minBigBlockSize;
    /// Max (and default) value of the big block size threshold.
    size_t maxBigBlockSize;
    /// Min allocation segment size.
    size_t minSegmentSize;
} QBBasicAllocatorConfig;


/**
 * Get compile-time configuration of the basic memory allocator.
 *
 * @return configuration of the memory allocator
 **/
extern const QBBasicAllocatorConfig *
QBBasicAllocatorGetConfig(void);


/**
 * Get runtime type identification object representing
 * basic memory allocator class.
 *
 * @return basic memory allocator class
 **/
extern SvType
QBBasicAllocator_getType(void);

/**
 * Initialize basic memory allocator object.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator object handle
 * @param[in] pool      memory pool to use for allocating memory segments
 *                      (handle to an object implementing QBMemPool interface)
 * @param[in] segmentSize unit of allocation for underlying allocation layer:
 *                      segments from @a pool will be allocated in multiplies
 *                      of @a segmentSize; must be >= QBBasicAllocatorConfig::minSegmentSize
 * @param[in] bigBlockSize size (in bytes) above which all allocated blocks
 *                      will be treated as "big" blocks (i.e. allocated
 *                      from dedicated memory pool segments); pass @c 0 to
 *                      turn off "big" block handling (every allocation is
 *                      treated like "small" block); must be <= QBBasicAllocatorConfig::maxBigBlockSize
 *                      and >= QBBasicAllocatorConfig::minBigBlockSize
 *                      and < @a segmentSize
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBBasicAllocator
QBBasicAllocatorInit(QBBasicAllocator self,
                     SvObject pool,
                     size_t segmentSize,
                     size_t bigBlockSize,
                     SvErrorInfo *errorOut);

/**
 * Setup allocator for usage from multiple threads.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator object handle
 * @param[out] errorOut error info
 **/
extern void
QBBasicAllocatorEnableLocking(QBBasicAllocator self,
                              SvErrorInfo *errorOut);

/**
 * Get memory usage statistics from allocator.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator handle
 * @param[out] stats    memory usage statistics
 * @param[out] segments array of segments allocated from the memory pool,
 *                      released by the caller (pass @c NULL if you don't care)
 * @param[out] blocks   array of descriptions of allocated blocks, use free()
 *                      after using it (pass @c NULL if you don't care)
 * @param[out] errorOut error info
 * @return              number of items in @a blocks array,
 *                      @c 0 if @a blocks is @c NULL,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBBasicAllocatorGetStats(QBBasicAllocator self,
                         QBBasicAllocatorStats *stats,
                         SvArray *segments,
                         QBBasicAllocatorBlocksListItem **blocks,
                         SvErrorInfo *errorOut);

/**
 * Get simplified memory usage statistics from allocator.
 *
 * This method is similar to QBBasicAllocatorGetStats(), but returns only
 * a subset of information available without accessing allocated memory
 * blocks. This way it is safe to use this method in emergency situations
 * when the underlying memory segments cannot be accessed anymore.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator handle
 * @param[out] stats    memory usage statistics
 * @param[out] errorOut error info
 **/
extern void
QBBasicAllocatorGetBasicStats(QBBasicAllocator self,
                              QBBasicAllocatorStats *stats,
                              SvErrorInfo *errorOut);

/**
 * Set limit on memory usage.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator object handle
 * @param[in] limit     max number of bytes in all allocated
 *                      memory blocks
 * @param[out] errorOut error info
 **/
extern void
QBBasicAllocatorSetLimit(QBBasicAllocator self,
                         size_t limit,
                         SvErrorInfo *errorOut);

/**
 * Allocate memory block.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator object handle
 * @param[in] size      desired block size in bytes
 * @param[out] errorOut error info
 * @return              allocated block aligned to a multiple of 16 bytes,
 *                      @c NULL in case of error
 **/
extern void *
QBBasicAllocatorAllocate(QBBasicAllocator self,
                         size_t size,
                         SvErrorInfo *errorOut);

/**
 * Allocate memory block with extra requirements for its properties.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator object handle
 * @param[in] size      desired block size in bytes
 * @param[in] alignment desired block alignment in bytes
 * @param[in] flags     extra block properties (reserved for future
 *                      extensions, pass @c 0 here)
 * @param[out] errorOut error info
 * @return              allocated block, @c NULL in case of error
 **/
extern void *
QBBasicAllocatorAllocateSpecial(QBBasicAllocator self,
                                size_t size,
                                size_t alignment,
                                unsigned int flags,
                                SvErrorInfo *errorOut);

/**
 * Reallocate memory block.
 *
 * This method modifies the size of allocated block. It behaves
 * just like standard realloc() function: returned block is
 * either the same block (if it could be enlarged) or new block
 * with copy of the old one's contents.
 *
 * @note All special attributes of the block (alignment and flags)
 *       are preserved by this method.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator object handle
 * @param[in] ptr       previously allocated block or @c NULL
 * @param[in] size      desired block size in bytes
 * @param[out] errorOut error info
 * @return              reallocated block aligned to a multiple of 16 bytes,
 *                      @c NULL in case of error
 **/
extern void *
QBBasicAllocatorReallocate(QBBasicAllocator self,
                           void *ptr,
                           size_t size,
                           SvErrorInfo *errorOut);

/**
 * Free allocated memory block.
 *
 * @memberof QBBasicAllocator
 *
 * @param[in] self      basic memory allocator object handle
 * @param[in] ptr       previously allocated block or @c NULL
 **/
extern void
QBBasicAllocatorFree(QBBasicAllocator self,
                     void *ptr);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_ALLOCATOR_H_
#define SV_ALLOCATOR_H_

/**
 * @file SvAllocator.h Memory allocator interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvCoreMemoryManagement Memory management
 * @ingroup SvCore
 *
 * Memory allocator interface and available implementations.
 **/

/**
 * @defgroup SvAllocator Memory allocator
 * @ingroup SvCoreMemoryManagement
 * @{
 **/

/// Max number of memory allocation zones.
#define SV_ALLOCATOR_MAX_ZONES_COUNT        8192

/**
 * Memory allocation zone.
 **/
typedef struct SvAllocatorZone_ {
    /// name of the zone, @c NULL for default zone
    const char *name;
    /// zone identifier filled by SvAllocatorPushZone()
    unsigned long int ID;
} SvAllocatorZone;

/**
 * Memory allocation zone statistics.
 **/
typedef struct SvAllocatorZoneStats_ {
    /// zone identifier
    unsigned int ID;
    /// zone name
    const char *name;
    /// @c true if full allocation tracing is enabled for this zone
    bool tracingEnabled;
    /// number of blocks allocated from this zone
    size_t blocksCount;
    /// total memory used by all blocks in this zone (in bytes)
    size_t totalUsed;
} SvAllocatorZoneStats;

/**
 * Type of a single memory allocation.
 **/
typedef enum {
    /// default block type
    SvAllocatorBlockType_default = 0,
    /**
     * block allocated directly with malloc() or other function
     * from standard library
     **/
    SvAllocatorBlockType_malloc = SvAllocatorBlockType_default,
    /// block allocated with SvAllocator
    SvAllocatorBlockType_allocator,
    /// block containing SvObject
    SvAllocatorBlockType_object,
} SvAllocatorBlockType;

/**
 * Memory allocation block statistics.
 **/
typedef struct SvAllocatorBlockStats_ {
    /// memory block
    void *ptr;
    /// allocation type
    SvAllocatorBlockType type;
    /// number of valid zone identifiers in @a zonesStack, at least @c 1
    unsigned int stackDepth;
#define SV_ALLOCATOR_ZONES_STACK_MAX_SIZE 6
    /// stack of allocation zones this block belongs to
    unsigned short int zonesStack[SV_ALLOCATOR_ZONES_STACK_MAX_SIZE];
} SvAllocatorBlockStats;

/**
 * The SvAllocator type.
 *
 * @a note SvAllocator is @b not an SvObject.
 **/
typedef struct SvAllocator_ {
    /**
     * Allocate a block of memory in current memory zone.
     *
     * @param[in] size      requested block size in bytes
     * @return              allocated block, @c NULL in case of error
     **/
    void *(*alloc)(size_t size);

    /**
     * Allocate aligned block of memory in current memory zone.
     *
     * @param[in] size      requested block size in bytes
     * @param[in] alignment memory address alignment
     * @return              allocated block, @c NULL in case of error
     **/
    void *(*allocAligned)(size_t size, size_t alignment);

    /**
     * Deallocate memory.
     *
     * @param[in] ptr       a pointer to previously allocated block of memory
     *                      or @c NULL in case of error
     **/
    void (*free)(void *ptr);

    /**
     * Resize previously allocated block of memory keeping it
     * in the original memory zone.
     *
     * @param[in] ptr       a pointer to previously allocated memory or @c NULL
     * @param[in] size      requested new size of the memory block
     * @return              a pointer to the reallocated memory
     *                      or @c NULL in case of error
     **/
    void *(*realloc)(void *ptr, size_t size);

    /**
     * Allocate a block of memory in current memory zone and fill it with zeros.
     *
     * @param[in] size      size in bytes of a single element to be allocated
     * @param[in] n         number of elements to allocate
     * @return              allocated block, @c NULL in case of error
     **/
    void *(*calloc)(size_t size, size_t n);

    /**
     * Allocate a block of memory in current memory zone and mark
     * it as being used by an object.
     *
     * @param[in] size      requested block size in bytes
     * @return              allocated block, @c NULL in case of error
     **/
    void *(*allocObject)(size_t size);

    /**
     * Allocate aligned block of memory in the same memory zone as one
     * of previously allocated blocks.
     *
     * @param[in] size      requested block size in bytes
     * @param[in] alignment memory address alignment
     * @param[in] ptr       previously allocated block to get zone information from
     * @return              allocated block, @c NULL in case of error
     **/
    void *(*allocLinked)(size_t size, size_t alignment, const void *ptr);

    /**
     * Push allocation zone on the stack of current thread.
     *
     * @param[in] zone      allocation zone
     * @return              @c 0 on success, @c -1 if per-zone allocation
     *                      tracing is not supported by this allocator or disabled
     **/
    int (*pushZone)(SvAllocatorZone *const zone);

    /**
     * Pop allocation zone from the stack of current thread.
     *
     * @return              @c 0 on success, @c -1 if per-zone allocation
     *                      tracing is not supported by this allocator or disabled
     **/
    int (*popZone)(void);

    /**
     * Get information about allocation zones.
     *
     * @param[in] stats     array of zone statistics to fill
     * @param[in] n         number of elements in @a stats array
     * @return              number of registered zones (can be greater than @a n),
     *                      @c -1 if per-zone allocation tracing is not supported
     *                      by this allocator or disabled
     **/
    ssize_t (*getZoneStats)(SvAllocatorZoneStats *stats, size_t n);

    /**
     * Get information about allocated memory blocks in given allocation zone.
     *
     * @param[in] zoneID    zone identifier
     * @param[in] stats     array of block statistics to fill
     * @param[in] n         number of elements in @a stats array
     * @return              number of blocks tracked in this zone (can be greater
     *                      than @a n), @c -1 if per-zone allocation tracing
     *                      is not supported by this allocator or disabled
     **/
    ssize_t (*getBlockStats)(unsigned int zoneID, SvAllocatorBlockStats *stats, size_t n);
} *SvAllocator;


/**
 * Allocate arbitrary amount of memory in current memory zone.
 *
 * This function is an SvCore-backed malloc replacement.
 * Allocated memory is always zero-filled, like in calloc().
 *
 * The allocation itself is performed by the given allocator.
 * Typical usage simply uses SvStdAllocator to get standard library's
 * malloc() implementation. Other, specialized allocators may be deployed
 * if necessary.
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] size      requested amount of memory in bytes. May be @c 0 only
 *                      if the allocator actually supports this value. In general
 *                      the size must be supported by the allocator used.
 * @param[out] errorOut error info
 * @return              a pointer to newly allocated memory or @c NULL if
 *                      the allocation could not be performed
 **/
extern void *
SvAllocatorAllocate(SvAllocator self,
                    size_t size,
                    SvErrorInfo *errorOut);

/**
 * Allocate aligned memory in current memory zone.
 *
 * This function allocates @a size bytes of memory.
 * Returned pointer is aligned to the multiple of @a alignement value.
 * If the alignment request cannot be fulfilled, allocation fails.
 * Allocated memory is \b not zero-filled!
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] size      requested amount of memory in bytes
 * @param[in] alignment memory address alignment
 * @param[out] errorOut error info
 * @return              a pointer to newly allocated, properly aligned
 *                      memory block or @c NULL if the allocation
 *                      could not be performed
 **/
extern void *
SvAllocatorAllocateAligned(SvAllocator self,
                           size_t size,
                           size_t alignment,
                           SvErrorInfo *errorOut);

/**
 * Allocate aligned memory block in the same allocation zone as existing one.
 *
 * This function is an extended version of SvAllocatorAllocateAligned().
 * If @a ptr is specified, allocated block will be assigned to the same
 * allocation zone as block at @a ptr. If @a ptr is @c NULL, this function
 * behaves exactly like SvAllocatorAllocateAligned().
 *
 * @since 1.12
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] size      requested amount of memory in bytes
 * @param[in] alignment memory address alignment
 * @param[in] ptr       memory block identifying allocation zone to add
 *                      newly allocated block to
 * @param[out] errorOut error info
 * @return              a pointer to newly allocated, properly aligned
 *                      memory block or @c NULL if the allocation
 *                      could not be performed
 **/
extern void *
SvAllocatorAllocateLinked(SvAllocator self,
                          size_t size,
                          size_t alignment,
                          const void *ptr,
                          SvErrorInfo *errorOut);

/**
 * Allocate memory to be used for object in current memory zone.
 *
 * This function allocates memory to be used for object and fills it with zero
 * bytes. If given allocator does not support special mode for allocating
 * memory for objects, it works exactly like SvAllocatorAllocate().
 *
 * @since 1.12
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] size      object size in bytes
 * @param[out] errorOut error info
 * @return              a pointer to newly allocated memory or @c NULL if
 *                      the allocation could not be performed
 **/
extern void *
SvAllocatorAllocateObject(SvAllocator self,
                          size_t size,
                          SvErrorInfo *errorOut);

/**
 * Deallocate memory.
 *
 * This function frees memory previously allocated with SvAllocatorAllocate(),
 * SvAllocatorAllocateAligned() or SvAllocatorAllocateObject().
 * The same allocator object must be used in both invocations.
 * As a special case any allocator can free a @c NULL pointer.
 *
 * @param[in] self      reference to an allocator. @c NULL to use default one
 * @param[in] ptr       a pointer to previously allocated memory or @c NULL
 **/
extern void
SvAllocatorDeallocate(SvAllocator self,
                      void *ptr);

/**
 * Resize memory block previously allocated with SvAllocatorAllocate()
 * or SvAllocatorAllocateAligned().
 *
 * This function resizes memory block previously allocated with
 * SvAllocatorAllocate() or SvAllocatorAllocateAligned(). The same allocator
 * object must be used in both invocations.
 *
 * As a special case any allocator can resize a @c NULL pointer, in that case
 * it behaves exactly like SvAllocatorAllocate().
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] ptr       a pointer to previously allocated memory or @c NULL
 * @param[in] size      requested new size of the memory block. May be @c 0
 *                      only if the allocator actually supports this value.
 *                      In general the size must be supported by the used
 *                      allocator.
 * @param[out] errorOut error info
 * @return              a pointer to the reallocated memory or @c NULL in
 *                      case of error
 **/
extern void *
SvAllocatorReallocate(SvAllocator self,
                      void *ptr,
                      size_t size,
                      SvErrorInfo *errorOut);

/**
 * Create a copy of the given memory region allocated in current memory zone.
 *
 * @since 1.4
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] ptr       pointer to the memory region to copy
 * @param[in] size      size of the memory region to copy
 * @param[out] errorOut error info
 * @return              pointer to the copied region on success
 *                      or @c NULL in case of error
 **/
extern void *
SvAllocatorCopyMemory(SvAllocator self,
                      const void *ptr,
                      size_t size,
                      SvErrorInfo *errorOut);

/**
 * Create a copy of the given C string allocated in current memory zone.
 *
 * @since 1.4
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] cstr      NULL-terminated string
 * @param[out] errorOut error info
 * @return              pointer to the copied string on success
 *                      or @c NULL in case of error
 **/
extern void *
SvAllocatorCopyString(SvAllocator self,
                      const char *cstr,
                      SvErrorInfo *errorOut);

/**
 * Push allocation zone on the stack of current thread.
 *
 * @since 1.12
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] zone      allocation zone
 * @return              @c 0 on success, @c -1 if per-zone allocation
 *                      tracing is not supported by this allocator or disabled
 **/
extern int
SvAllocatorPushZone(SvAllocator self,
                    SvAllocatorZone *const zone);

/**
 * Pop allocation zone from the stack of current thread.
 *
 * @since 1.12
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @return              @c 0 on success, @c -1 if per-zone allocation
 *                      tracing is not supported by this allocator or disabled
 **/
extern int
SvAllocatorPopZone(SvAllocator self);

/**
 * Get information about allocation zones.
 *
 * @since 1.12
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] stats     array of zone statistics to fill
 * @param[in] n         number of elements in @a stats array
 * @return              number of registered zones (can be greater than @a n),
 *                      @c -1 if per-zone allocation tracing is not supported
 *                      by this allocator or disabled
 **/
extern ssize_t
SvAllocatorGetZonesStats(SvAllocator self,
                         SvAllocatorZoneStats *stats,
                         size_t n);

/**
 * Get information about allocated memory blocks in given allocation zone.
 *
 * @since 1.12
 *
 * @param[in] self      reference to an allocator, @c NULL to use default one
 * @param[in] zoneID    zone identifier
 * @param[in] stats     array of block statistics to fill
 * @param[in] n         number of elements in @a stats array
 * @return              number of blocks tracked in this zone (can be greater
 *                      than @a n), @c -1 if per-zone allocation tracing
 *                      is not supported by this allocator or disabled
 **/
extern ssize_t
SvAllocatorGetBlockStats(SvAllocator self,
                         unsigned int zoneID,
                         SvAllocatorBlockStats *stats,
                         size_t n);

/**
 * @}
 **/


/**
 * @defgroup SvCoreMemoryAllocators Available memory allocators
 * @ingroup SvCoreMemoryManagement
 * @{
 **/

/**
 * Default memory allocator.
 **/
#define SvDefaultAllocator ((void *) 0)

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

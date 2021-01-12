/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
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

#ifndef SV_GFX_QUEUE_H_
#define SV_GFX_QUEUE_H_

/**
 * @file SvGfxQueue.h Queue of GFX operations
 **/

#include <CAGE/Core/Sv2DRect.h>
#include <SvGfxHAL/SvGfxObject.h>
#include <SvGfxHAL/Sv3DOperation.h>
#include <SvGfxHAL/SvColorOperation.h>

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvGfxQueue Queue of GFX operations
 * @ingroup SvGfxHAL
 * @{
 *
 * Container that groups GFX operations for sending them to
 * accelerated GFX engine as one batch job.
 **/

/**
 * Types of GFX queue entries.
 **/
typedef enum {
    SvGfxQueueEntryType_none,                  ///< empty GFX entry
    SvGfxQueueEntryType_object,                ///< GFX object
    SvGfxQueueEntryType_3DOperation,           ///< 3D GFX operation
    SvGfxQueueEntryType_colorOperation,        ///< color operation
} SvGfxQueueEntryType;

/**
 * GFX queue entry definition.
 **/
typedef struct SvGfxQueueEntry_s {
    SvGfxQueueEntryType type;              ///< queue entry type
    union {                                ///< union with possible types of queue entries
        SvGfxObject      object;           ///< gfx object to handle
        Sv3DOperation    operation3D;      ///< 3d operation to handle
        SvColorOperation colorOperation;   ///< color operation to handle
    };
} SvGfxQueueEntry;

/**
 * Queue of GFX objects (operations).
 **/
typedef struct SvGfxQueue_s {
    /** number of allocated entries */
    size_t max;
    /** index of next free entry */
    uint32_t cnt;

    /** entries */
    SvGfxQueueEntry* entries;

    /** bounding destination rectangle of all operations in the queue */
    Sv2DRect boundingRect;
} SvGfxQueue;


/**
 * Initialize GFX queue.
 *
 * @param[in] queue     GFX queue handle
 **/
static inline void
SvGfxQueueInit(SvGfxQueue* queue)
{
    memset(queue, 0, sizeof(*queue));
}

/**
 * Deinitialize GFX queue (free internal storage).
 *
 * @param[in] queue     GFX queue handle
 **/
extern void
SvGfxQueueDeinit(SvGfxQueue* queue);

/**
 * Allocate more space in the GFX queue if needed.
 *
 * @param[in] queue        GFX queue handle
 * @param[in] minEntries   number of entries, that must fit in the queue
 * @return                 @c 0 on success, @c -1 otherwise
 **/
extern int
SvGfxQueueGrow(SvGfxQueue* queue,
               size_t minEntries);

/**
 * Copy contents of a GFX queue.
 *
 * @param[in] queueIn      source GFX queue handle
 * @param[in] queueOut     destination GFX queue handle
 **/
extern void
SvGfxQueueCopy(const SvGfxQueue* queueIn,
               SvGfxQueue* queueOut);

/**
 * Copy contents of a GFX queue with destination rectangle clipping.
 *
 * This method copies all operations from @a queueIn to @a queueOut,
 * clipping destination rectangles to @a clipRect.
 *
 * @param[in] queueIn      source GFX queue handle
 * @param[in] queueOut     destination GFX queue handle
 * @param[in] clipRect     clipping rectange
 * @return 0 on success, < 0 on failure
 **/
extern int
SvGfxQueueClip(const SvGfxQueue* queueIn,
               SvGfxQueue* queueOut,
               const Sv2DRect* const clipRect);

/**
 * Calculate bounding rectangle of all operations in the queue.
 *
 * @param[in] queue        GFX queue handle
 **/
extern void
SvGfxQueueCalcBoundingRect(SvGfxQueue* queue);

/**
 * Eliminate operations (or parts of operations) that don't have
 * any effect after rendering entire queue.
 *
 * @param[in] queue        GFX queue handle
 **/
extern void
SvGfxQueueOptimize(SvGfxQueue* queue);

/**
 * Calculate number of pixels written by GFX engine during
 * rendering of entire queue.
 *
 * @param[in] queue        GFX queue handle
 * @return                 number of written pixels
 **/
extern unsigned int
SvGfxQueueCalcOutputPixels(const SvGfxQueue* queue);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

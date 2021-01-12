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

#ifndef SVGFXONVIDEOQUEUE_H
#define SVGFXONVIDEOQUEUE_H

/**
 * @file SvGfxOnVideoQueue.h Queue for GFX object which must be relative to video output
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

#include <SvGfxHAL/SvGfxObject.h>
#include <SvGfxHAL/SvGfxQueue.h>

#include <QBPlatformHAL/QBPlatformViewport.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup SvGfxHAL
 * @{
 * Container for SvGfxObjects which have to be displayed with respect to
 * specific video output coordinates. When added to this queue, object's position and
 * size are recalculated according to video content coordinates on specific output.
 **/

/**
 * SvGfxOnVideoQueue type
 **/
typedef struct SvGfxOnVideoQueue_* SvGfxOnVideoQueue;

/**
 * Type of object in queue. This affects repositioning and
 * scaling GFX objects on output.
 **/
typedef enum {
    SvGfxOnVideoQueueObjectType_default,          /**< default calculation (no repositioning, no minimal size) */
    SvGfxOnVideoQueueObjectType_ConaxFingerprint, /**< calculation according to Conax fingerprint requirements */
} SvGfxOnVideoQueueObjectType;

/**
 * SvGfxOnVideoQueue constructor.
 *
 * @param[in] outputsMax maximum number of different outputs handled by queue
 * @param[in] objectsPerOutputMax maximum number of objects per output
 * @return object instance or NULL if failed
 */
extern SvGfxOnVideoQueue
SvGfxOnVideoQueueCreate(size_t outputsMax,
                        size_t objectsPerOutputMax);

/** Get runtime type identification object representing SvGfxOnVideoQueue class
 * @return object representing SvGfxOnVideoQueue class
 */
extern SvType
SvGfxOnVideoQueue_getType(void);

/**
 * Adds SvGfxObject to queue.
 *
 * Coordinates of @a obj are calculated relative to 720x576 canvas.
 *
 * @param[in] self instance of SvGfxOnVideoQueue
 * @param[in] outputID this must be one of masterGfxIds from platform HAL
 * @param[in] obj GFX object we want to be displayed on output
 * @param[in] minimalSize minimal size (used in calculations for specific @a type)
 * @param[in] type of GFX object
 * @param[out] handle for object
 * @return 0 on success, -1 if fail
 */
extern int
SvGfxOnVideoQueueAddObject(SvGfxOnVideoQueue self,
                           unsigned int outputID,
                           SvGfxObject *obj,
                           const Sv2DRect *minimalSize,
                           SvGfxOnVideoQueueObjectType type,
                           unsigned int *handle);

/**
 * Removes SvGfxObject from queue
 *
 * @param[in] self instance of SvGfxOnVideoQueue
 * @param[in] outputID this must be one of masterGfxIds from platform HAL
 * @param[out] handle for object to be removed
 * @return 0 on success, -1 if fail
 */
extern int
SvGfxOnVideoQueueRemoveObject(SvGfxOnVideoQueue self,
                              unsigned int outputID,
                              unsigned int handle);

/**
 * Sets additional scaling rectangle. If set, coordinates of
 * objects will be rescaled to provided @a fbRect
 *
 * @param[in] self instance of SvGfxOnVideoQueue
 * @param[in] outputID this must be one of masterGfxIds from platform HAL
 * @param[in] fbRect scalling rectangle, NULL if not used
 * @return 0 on success, -1 if fail
 */
extern int
SvGfxOnVideoQueueSetFBRect(SvGfxOnVideoQueue self,
                           unsigned int outputID,
                           Sv2DRect* fbRect);

/**
 * Returns pointer to SvGfxQueue of objects with recalculated positions and sizes.
 *
 * @param[in] self instance of SvGfxOnVideoQueue
 * @param[in] outputID this must be one of masterGfxIds from platform HAL
 * @param[out] updatedSinceLastGet if not NULL it will tell if objects in queue changed since last get
 * @return pointer to SvGfxQueue
 */
extern const SvGfxQueue*
SvGfxOnVideoQueueGet(SvGfxOnVideoQueue self,
                     unsigned int outputID,
                     bool* updatedSinceLastGet);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // SVGFXONVIDEOQUEUE_H

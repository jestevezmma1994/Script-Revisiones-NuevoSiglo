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

#ifndef QB_PLATFORM_VIEWPORT_H_
#define QB_PLATFORM_VIEWPORT_H_

/**
 * @file QBPlatformViewport.h Viewport configuration management API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CAGE/Core/Sv2DRect.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformViewport Viewport configuration management
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Viewport type.
 **/
typedef enum {
    /// reserved value, used as marker for invalid/unknown type
    QBViewportID_invalid = 0,
    /// basic video viewport
    QBViewportID_video,
    /// graphics viewport
    QBViewportID_GFX,
} QBViewportID;

/**
 * Platform viewport info for HAL event callback.
 **/
typedef struct QBPlatformViewportInfo_ {
    /// parent object
    struct SvObject_ super;
    /// viewport ID
    QBViewportID viewportID;
    /// output id
    unsigned int outputID;
    /// viewport mode
    QBViewportMode mode;
    /// output rectangle
    Sv2DRect outputRect;
    /// input rectangle
    Sv2DRect inputRect;
}* QBPlatformViewportInfo;

/**
 * Record new viewport parameters.
 *
 * This method notifies platform HAL about change in viewport
 * parameters.
 *
 * Position of the viewport is configured separately for each master output,
 * using output rectangle in coordinate system of the output.
 * Platform HAL allows describing output rectangle of a video viewport
 * relative to GFX viewport (it will be automatically translated to
 * output coordinates for each output), but GFX viewport must not be
 * disabled for this to work.
 *
 * @param[in] viewportID   viewport ID
 * @param[in] outputID     ID of the output to configure viewport for,
 *                         @c -1 to configure relative to GFX viewport
 *                         (can be used only for video viewports)
 * @param[in] mode         new viewport mode
 * @param[in] outputRect   output rectangle, valid only
 *                         if @a mode == QBViewportMode_windowed
 * @param[in] inputRect    input rectangle, required for GFX viewport
 * @return                 @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformUpdateViewport(QBViewportID viewportID,
                         int outputID,
                         QBViewportMode mode,
                         const Sv2DRect *outputRect,
                         const Sv2DRect *inputRect);

/**
 * Record new viewport parameters without locking platform.
 *
 * This function do same as @ref QBPlatformUpdateViewport but,
 * does not lock platform and is used mainly in platform HALs,
 * where platform is already locked. Of course this is wrong approach.
 * This function should disappear in future.
 *
 * @param[in] viewportID   viewport ID
 * @param[in] outputID     ID of the output to configure viewport for,
 *                         @c -1 to configure relative to GFX viewport
 *                         (can be used only for video viewports)
 * @param[in] mode         new viewport mode
 * @param[in] outputRect   output rectangle, valid only
 *                         if @a mode == QBViewportMode_windowed
 * @param[in] inputRect    input rectangle, required for GFX viewport
 * @return                 @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformUpdateViewportNoLock(QBViewportID viewportID,
                               int outputID,
                               QBViewportMode mode,
                               const Sv2DRect *outputRect,
                               const Sv2DRect *inputRect);

/**
 * Get current viewport parameters.
 *
 * @param[in] viewportID   viewport ID
 * @param[in] outputID     ID of the output to get viewport parameters for
 * @param[out] mode        current viewport mode (@c NULL if you don't care)
 * @param[out] outputRect  output rectangle in output coordinates
 *                         (@c NULL if you don't care)
 * @param[out] inputRect   input rectangle in canvas coords
 *                         (@c NULL if you don't care)
 * @return                 @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformGetViewport(QBViewportID viewportID,
                      unsigned int outputID,
                      QBViewportMode *mode,
                      Sv2DRect *outputRect,
                      Sv2DRect *inputRect);

/**
 * Update inputRect for video viewport
 *
 * @param outputID     ID of the output to get viewport parameters for
 * @param inputRect    input rectangle in canvas coords, if NULL, it will be calculated from following params
 * @param mode         input content mode, ignored when inputRect != NULL
 * @param aspectRatio  input content aspectRatio, ignored when inputRect != NULL
 * @return
 */
extern int
QBPlatformUpdateVideoViewportInput(unsigned int outputID,
                                   Sv2DRect *inputRect,
                                   QBContentDisplayMode mode,
                                   QBAspectRatio aspectRatio);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

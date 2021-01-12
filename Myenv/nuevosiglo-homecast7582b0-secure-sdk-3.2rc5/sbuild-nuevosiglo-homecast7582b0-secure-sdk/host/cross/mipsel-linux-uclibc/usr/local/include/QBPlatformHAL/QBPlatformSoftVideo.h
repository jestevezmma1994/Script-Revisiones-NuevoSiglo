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

#ifndef QB_PLATFORM_HAL_SOFT_VIDEO_H_
#define QB_PLATFORM_HAL_SOFT_VIDEO_H_

#include <CAGE/Core/SvBitmap.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformSoftVideo Software video frames upload
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Get free frame to render to.
 *
 * @param[in] width         width of the viewport source frames in pixels
 * @param[in] height        height of the viewport source frames in pixels
 * @param[in] csp           color space of the viewport source frames
 * @return                  handle to a frame to render to,
 *                          @c NULL if not available
 **/
extern SvBitmap
SoftVideoGetViewportFrame(unsigned int width,
                          unsigned int height,
                          SvColorSpace csp);

/**
 * Aspect ratio of a video frame.
 **/
typedef struct {
    /// aspect ratio nominator value
    unsigned int nominator;
    /// aspect ratio denominator value
    unsigned int denominator;
} SoftVideoFrameAspectRatio;

/**
 * Push filled frame to be displayed in video viewport.
 *
 * @param[in] frame         handle to a frame received previously from
 *                          SoftVideoGetViewportFrame()
 * @param[in] aspectRatio   real aspect ratio of the frame
 * @return                  @c on success, @c -1 on case of error
 **/
extern int
SoftVideoPushViewportFrame(SvBitmap frame,
                           SoftVideoFrameAspectRatio aspectRatio);
/**
 * @cond
 **/
extern SvBitmap
SoftVideoTakeViewportFrameForDisplay(bool* clearOut);
/**
 * @endcond
 **/

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

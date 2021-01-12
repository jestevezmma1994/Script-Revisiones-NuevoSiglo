/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef XMB_ICON_H_
#define XMB_ICON_H_

/**
 * @file XMBIcon.h
 * @brief Xross Media Bar icon widget API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup XMBIcon XMB icon widget
 * @ingroup XMB
 * @{
 *
 * Widget implementing XMB menu bar icon.
 **/

/**
 * XMB icon bitmap descriptor.
 **/
struct XMBIconBitmap_ {
    /// bitmap to be shown
    SvBitmap bmp;
    /// bitmap scale when this bitmap is inactive
    float inactiveScale;
    /// bitmap scale when this bitmap is active
    float activeScale;
    /// horizontal position of the center point of orginal bitmap, 0 <= xCenterRatio <= 1
    double xCenterRatio;
    /// vertical position of the center point of orginal bitmap, 0 <= yCenterRatio <= 1
    double yCenterRatio;
    /// how many pixels from left side was cut from orginal bitmap
    int offsetX;
    /// how many pixels from top was cut from orginal bitmap
    int offsetY;
    /// orginal width of bitmap
    int orgWidth;
    /// orginal height of bitmap
    int orgHeight;
};


/**
 * Create new XMB icon widget.
 *
 * @param[in] app           CUIT application handle
 * @param[in] width         icon width in pixels
 * @param[in] height        icon height in pixels
 * @param[in] bitmaps       array of two bitmaps
 * @param[out] errorOut     error info
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
XMBIconNew(SvApplication app,
           unsigned int width,
           unsigned int height,
           const struct XMBIconBitmap_ *bitmaps,
           SvErrorInfo *errorOut);

/**
 * Get index of currently selected bitmap.
 *
 * @param[in] icon          XMB icon widget handle
 * @return                  index of selected bitmap, @c -1 if not available
 **/
extern int
XMBIconGetSelectedBitmap(SvWidget icon);

/**
 * Select active bitmap to be shown by XMB icon.
 *
 * @param[in] icon          XMB icon widget handle
 * @param[in] index         index of the bitmap to be shown (@c 1 or @c 2),
 *                          @c 0 to hide both bitmaps
 * @param[in] switchDuration duration of the bitmap switch
 * @param[out] errorOut     error info
 **/
extern void
XMBIconSelectBitmap(SvWidget icon,
                    unsigned int index,
                    double switchDuration,
                    SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif

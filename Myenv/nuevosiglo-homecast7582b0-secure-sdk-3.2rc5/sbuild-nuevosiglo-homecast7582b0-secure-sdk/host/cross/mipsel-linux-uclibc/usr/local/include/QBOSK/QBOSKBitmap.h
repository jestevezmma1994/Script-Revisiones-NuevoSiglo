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

#ifndef QB_OSK_BITMAP_H_
#define QB_OSK_BITMAP_H_

/**
 * @file QBOSKBitmap.h
 * @brief On Screen Keyboard bitmap class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBOSKBitmap On Screen Keyboard bitmap class
 * @ingroup QBOSKCore
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/Sv2DRect.h>
#include <QBOSK/QBOSKTypes.h>


/**
 * Get runtime type identification object representing
 * OSK bitmap class.
 *
 * @return QBOSKBitmap type identification object
 **/
extern SvType
QBOSKBitmap_getType(void);

/**
 * Create new OSK bitmap.
 *
 * @param[in] width     bitmap width in pixels
 * @param[in] height    bitmap height in pixels
 * @param[in] csp       color space
 * @param[out] errorOut error info
 * @return              created OSK bitmap, @c NULL in case of error
 **/
extern QBOSKBitmap
QBOSKBitmapCreate(unsigned int width, unsigned int height,
                  SvColorSpace csp,
                  SvErrorInfo *errorOut);

/**
 * Compress region of OSK bitmap.
 *
 * @param[in] self      OSK bitmap handle
 * @param[in] rect      rectangle specifying which part of bitmap
 *                      is to be compressed, @c NULL for entire bitmap
 * @param[out] errorOut error info
 **/
extern void
QBOSKBitmapCompress(QBOSKBitmap self,
                    const Sv2DRect *rect,
                    SvErrorInfo *errorOut);

/**
 * Blit region of OSK bitmap to hardware surface.
 *
 * This method copies the specified region of the bitmap
 * to the hardware surface associated with it.
 * Bitmap data is automatically uncompressed beforehand.
 *
 * @param[in] self      OSK bitmap handle
 * @param[in] rect      rectangle specifying which part of bitmap
 *                      is to be copied, @c NULL for entire bitmap
 * @param[out] errorOut error info
 **/
extern void
QBOSKBitmapBlit(QBOSKBitmap self,
                const Sv2DRect *rect,
                SvErrorInfo *errorOut);

/**
 * Drop all uncompressed data kept by OSK bitmap.
 *
 * This method frees all uncompressed pixel data,
 * from both host and hardware (GFX) memory.
 *
 * @param[in] self      OSK bitmap handle
 * @param[out] errorOut error info
 **/
extern void
QBOSKBitmapFlush(QBOSKBitmap self,
                 SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif

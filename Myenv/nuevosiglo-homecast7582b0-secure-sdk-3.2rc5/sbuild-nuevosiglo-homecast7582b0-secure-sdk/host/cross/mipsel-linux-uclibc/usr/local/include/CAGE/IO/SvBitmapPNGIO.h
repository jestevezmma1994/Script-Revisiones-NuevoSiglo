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

#ifndef CAGE_BITMAP_PNG_IO_H_
#define CAGE_BITMAP_PNG_IO_H_

/**
 * @file SvBitmapPNGIO.h Bitmap PNG input/output API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvData.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvBitmap.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup SvBitmap_IO
 * @{
 **/

/**
 * Create host bitmap from PNG file.
 *
 * This method creates new host bitmap from PNG file.
 *
 * @note This operation is synchronous, so it should not be used
 *       in GUI applications; use Resource Manager to load bitmaps.
 *       This method is provided only for very simple applications
 *       like upgrade progress GUI.
 *
 * @memberof SvBitmap
 *
 * @param[in] filePath  path to PNG file
 * @param[out] errorOut error info
 * @return              newly created bitmap or @c NULL in case of error
 **/
extern SvBitmap
SvBitmapCreateFromPNG(const char *filePath,
                      SvErrorInfo *errorOut);

/**
 * Create host bitmap from PNG file data.
 *
 * This method creates new host bitmap from PNG file loaded into memory.
 *
 * @note This operation is synchronous, so it should not be used
 *       in GUI applications; use Resource Manager to load bitmaps.
 *       This method is provided only for very simple applications
 *       like upgrade progress GUI.
 *
 * @memberof SvBitmap
 *
 * @param[in] data      contents of a PNG file loaded into memory
 * @param[out] errorOut error info
 * @return              newly created bitmap or @c NULL in case of error
 **/
SvBitmap
SvBitmapCreateFromPNGData(SvData data,
                          SvErrorInfo *errorOut);

/**
 * Create host bitmap from PNG file stream.
 *
 * This method creates new host bitmap from PNG file stream.
 *
 * @note This operation is synchronous, so it should not be used
 *       in GUI applications; use Resource Manager to load bitmaps.
 *       This method is provided only for very simple applications
 *       like upgrade progress GUI.
 *
 * @memberof SvBitmap
 *
 * @param[in] stream    PNG file stream
 * @param[out] errorOut error info
 * @return              newly created bitmap or @c NULL in case of error
 */
SvBitmap
SvBitmapCreateFromPNGFileStream(FILE *stream,
                                SvErrorInfo *errorOut);

/**
 * Write host bitmap to PNG file.
 *
 * @note This operation is synchronous, so it should not be used
 *       in GUI applications.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle (source)
 * @param[in] srcRect   source region in a @a src bitmap,
 *                      @c NULL to write entire bitmap
 * @param[in] filePath  destination PNG file path
 * @param[out] errorOut error info
 **/
extern void
SvBitmapWriteToPNG(SvBitmap self,
                   const Sv2DRect *srcRect,
                   const char *filePath,
                   SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

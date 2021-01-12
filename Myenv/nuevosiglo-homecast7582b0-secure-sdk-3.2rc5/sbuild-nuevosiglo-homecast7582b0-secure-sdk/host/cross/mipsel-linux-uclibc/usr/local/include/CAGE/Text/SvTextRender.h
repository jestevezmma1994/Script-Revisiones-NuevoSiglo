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

#ifndef CAGE_TEXT_RENDER_H_
#define CAGE_TEXT_RENDER_H_

/**
 * @file SvTextRender.h Bitmap class interface for drawing text
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvTextLayout.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvBitmap_textDrawing Drawing text
 * @ingroup SvBitmap
 * @{
 **/

/**
 * Draw single line from text layout on a bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] baselineX X coordinate of the baseline start point
 * @param[in] baselineY Y coordinate of the baseline start point
 * @param[in] line      single line of a computed text layout
 * @param[out] errorOut error info
 **/
extern void
SvBitmapDrawTextLine(SvBitmap self,
                     int baselineX, int baselineY,
                     const SvTextLayoutLine *line,
                     SvErrorInfo *errorOut);

/**
 * Draw text layout on a bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] baselineX X coordinate of the left most line's baseline start point
 * @param[in] baselineY Y coordinate of the first line's baseline start point
 * @param[in] layout    computed text layout
 * @param[out] errorOut error info
 **/
extern void
SvBitmapDrawTextLayout(SvBitmap self,
                       int baselineX, int baselineY,
                       SvTextLayout layout,
                       SvErrorInfo *errorOut);

/**
 * @}
 **/

/**
 * @addtogroup SvBitmap_create
 * @{
 **/

/**
 * Create new host bitmap in @ref SvColorSpace::SV_CS_MONO color space
 * with left-aligned text rendered on it.
 *
 * @note To set font size and/or aspect use SvFontSetSize()
 *       and/or SvFontSetAspect() before calling this method.
 *
 * @memberof SvBitmap
 *
 * @param[in] text      text in UTF-8 encoding to be rendered on a bitmap
 * @param[in] font      handle to a font to render text in
 * @param[in] color     text color
 * @param[out] boundingBox bounding box of a text layout
 *                      (see SvTextLayoutProcessLines())
 * @return              new bitmap, @c NULL in case of error
 **/
extern SvBitmap
SvBitmapCreateWithText(const char *text,
                       SvFont font, SvColor color,
                       Sv2DRect *boundingBox);

/**
 * Create new host bitmap in @ref SvColorSpace::SV_CS_MONO color space
 * with text rendered on it.
 *
 * @note To set font size and/or aspect use SvFontSetSize()
 *       and/or SvFontSetAspect() before calling this method.
 *
 * @memberof SvBitmap
 *
 * @param[in] text      text in UTF-8 encoding to be rendered on a bitmap
 * @param[in] font      handle to a font to render text in
 * @param[in] color     text color
 * @param[in] maxWidth  max width of text box in pixels, @c 0 for no limit
 * @param[in] maxLines  max number of text lines in a box, @c 0 for no limit
 * @param[in] alignment text alignment
 * @param[out] boundingBox bounding box of a text layout
 *                      (see SvTextLayoutProcessLines())
 * @return              new bitmap, @c NULL in case of error
 **/
extern SvBitmap
SvBitmapCreateWithTextBox(const char *text,
                          SvFont font, SvColor color,
                          unsigned int maxWidth, unsigned int maxLines,
                          SvTextAlignment alignment,
                          Sv2DRect *boundingBox);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

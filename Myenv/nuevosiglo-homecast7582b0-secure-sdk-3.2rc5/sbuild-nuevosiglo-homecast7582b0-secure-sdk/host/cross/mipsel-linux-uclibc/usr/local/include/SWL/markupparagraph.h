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

#ifndef SWL_MARKUP_PARAGRAPH_
#define SWL_MARKUP_PARAGRAPH_

/**
 * @file SMP/SWL/swl/markupparagraph.h Markup text paragraph class API
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURI.h>
#include <SvFoundation/SvImmutableArray.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvTextTypes.h>
#include <CAGE/Text/SvFont.h>


/**
 * @defgroup SvMarkupParagraph Markup text paragraph class
 * @ingroup SWL
 * @{
 **/

/**
 * Markup text paragraph class.
 * @class SvMarkupParagraph
 * @extends SvObject
 **/
typedef struct SvMarkupParagraph_ *SvMarkupParagraph;

/**
 * Markup text paragraph image class.
 * @class SvMarkupParagraphImage
 * @extends SvObject
 **/
typedef struct SvMarkupParagraphImage_ {
    struct SvObject_ super_;
    unsigned int width, height;
    const char *uri;
    SvBitmap bmp;
} *SvMarkupParagraphImage;


/**
 * Get runtime type identification object representing
 * markup text paragraph class.
 *
 * @return markup text paragraph class
 **/
extern SvType
SvMarkupParagraph_getType(void);

/**
 * Create markup text paragraph object.
 *
 * @memberof SvMarkupParagraph
 *
 * @param[in] maxWidth      max paragraph width in pixels,
 *                          @c 0 for no width limit
 * @param[in] maxLines      max number of lines in a paragraph,
 *                          @c 0 for no width limit
 * @param[out] errorOut     error info
 * @return                  created markup paragraph object
 *                          or @c NULL in case of error
 **/
extern SvMarkupParagraph
SvMarkupParagraphCreate(unsigned int maxWidth,
                        unsigned int maxLines,
                        SvErrorInfo *errorOut);

/**
 * Set default text attributes.
 *
 * This method specifies how to render the text in all those parts
 * of the paragraph, where it is not described by the markup.
 *
 * @memberof SvMarkupParagraph
 *
 * @param[in] self          markup text paragraph handle
 * @param[in] font          handle to a default font
 * @param[in] fontSize      default font size
 * @param[in] textColor     default text color
 * @param[out] errorOut     error info
 **/
extern void
SvMarkupParagraphSetDefaultAttributes(SvMarkupParagraph self,
                                      SvFont font,
                                      unsigned int fontSize,
                                      SvColor textColor,
                                      SvErrorInfo *errorOut);

/**
 * Add a font to the markup paragraph.
 *
 * Markup paragraph maintains its own set of fonts, that are used
 * to render text marked with tags like '\<u\>' and '\<i\>'. Initially the only
 * available font is the one specified by SvMarkupParagraphSetDefaultAttributes().
 * This method adds another font to this set.
 *
 * @note This method must be used to add all required fonts
 * (ones that appear in the paragraph) before the paragraph
 * object is used to perform text layout.
 *
 * @memberof SvMarkupParagraph
 *
 * @param[in] self          markup text paragraph handle
 * @param[in] weight        weight of the @a font
 * @param[in] style         style of the @a font
 * @param[in] font          handle to a font to be used for rendering
 *                          text in given @a style and @a weight
 * @param[out] errorOut     error info
 **/
extern void
SvMarkupParagraphAddFont(SvMarkupParagraph self,
                         SvFontWeight weight,
                         SvFontStyle style,
                         SvFont font,
                         SvErrorInfo *errorOut);

/**
 * Get font used to render text.
 *
 * This method returns font that was used to render text.
 * If there were multiple fonts/sizes used, it reports error
 * and returns @c NULL.
 *
 * @memberof SvMarkupParagraph
 *
 * @param[in] self          markup text paragraph handle
 * @param[out] errorOut     error info
 * @return                  font used to render text,
 *                          @c NULL if multiple fonts were used
 **/
extern SvFont
SvMarkupParagraphGetFont(SvMarkupParagraph self,
                         SvErrorInfo *errorOut);

/**
 * Get color of the rendered text.
 *
 * This method returns color of the rendered text.
 * If there were multiple colors used, it reports error
 * and returns @c 0.
 *
 * @memberof SvMarkupParagraph
 *
 * @param[in] self          markup text paragraph handle
 * @param[out] errorOut     error info
 * @return                  color of the rendered text,
 *                          @c 0 if multiple colors were used
 **/
extern SvColor
SvMarkupParagraphGetTextColor(SvMarkupParagraph self,
                              SvErrorInfo *errorOut);

/**
 * Parse markup text.
 *
 * @note A markup text paragraph object is reusable: this method
 * can be called multiple times on a single instance, so it can
 * be used for rendering multiple paragraphs of text.
 *
 * @memberof SvMarkupParagraph
 *
 * @param[in] self          markup text paragraph handle
 * @param[in] markup        markup text to be parsed
 * @param[out] errorOut     error info
 **/
extern void
SvMarkupParagraphParse(SvMarkupParagraph self,
                       const char *markup,
                       SvErrorInfo *errorOut);

/**
 * Get all images present in markup text.
 *
 * @memberof SvMarkupParagraph
 *
 * @param[in] self          markup text paragraph handle
 * @param[out] errorOut     error info
 * @return                  an array of SvMarkupParagraphImage objects,
 *                          @c NULL in case of error
 **/
extern SvImmutableArray
SvMarkupParagraphGetImages(SvMarkupParagraph self,
                           SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif

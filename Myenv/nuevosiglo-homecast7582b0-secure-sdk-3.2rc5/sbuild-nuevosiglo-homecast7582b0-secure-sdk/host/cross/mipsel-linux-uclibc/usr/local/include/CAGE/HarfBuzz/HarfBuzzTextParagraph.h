/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef HARFBUZZ_TEXT_PARAGRAPH_
#define HARFBUZZ_TEXT_PARAGRAPH_

/**
 * @file HarfBuzzTextParagraph.h HarfBuzz text paragraph class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvFont.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup HarfBuzzTextParagraph HarfBuzz text paragraph class
 * @ingroup CAGE_TextRendering
 * @{
 *
 * HarfBuzzTextParagraph is an implementation of the @ref SvTextParagraph2
 * using HarfBuzz text shaping engine.
 *
 * @image html HarfBuzzTextParagraph.png
 **/


/**
 * HarfBuzz text paragraph class.
 * @class HarfBuzzTextParagraph
 * @extends SvAbstractTextParagraph
 **/
typedef struct HarfBuzzTextParagraph_ *HarfBuzzTextParagraph;


/**
 * Return the runtime type identification object
 * of the HarfBuzzTextParagraph class.
 *
 * @return HarfBuzz text paragraph class
 **/
extern SvType
HarfBuzzTextParagraph_getType(void);

/**
 * Create new HarfBuzz text paragraph.
 *
 * This is a convenience method that combines object allocation
 * and initialization.
 *
 * @memberof HarfBuzzTextParagraph
 *
 * @param[in] text      contents of the paragraph as UTF-8 string
 * @param[in] length    length of @a text, pass @c -1 to use entire text
 * @param[in] font      handle to a font to be used for @a text
 * @param[in] fontSize  size (in pixels) of font for @a text
 * @param[in] color     color to render @a text in
 * @param[out] errorOut error info
 * @return              new HarfBuzz text paragraph, @c NULL in case of error
 **/
extern HarfBuzzTextParagraph
HarfBuzzTextParagraphCreate(const char *text,
                            ssize_t length,
                            SvFont font,
                            unsigned int fontSize,
                            SvColor color,
                            SvErrorInfo *errorOut);

/**
 * Set script and language of the text.
 *
 * @param[in] self      HarfBuzz text paragraph handle
 * @param[in] script    ISO 15924 four letter script code, @c NULL to autodetect
 * @param[in] language  ISO 639-3 three letter language code, @c NULL to autodetect
 * @param[out] errorOut error info
 **/
extern void
HarfBuzzTextParagraphSetTextProperties(HarfBuzzTextParagraph self,
                                       const char *script,
                                       const char *language,
                                       SvErrorInfo *errorOut);

/**
 * Set list of OpenType features to enable in HarfBuzz shaper.
 *
 * @param[in] self      HarfBuzz text paragraph handle
 * @param[in] features  NULL-terminated array of feature tags
 * @param[out] errorOut error info
 **/
extern void
HarfBuzzTextParagraphSetOpenTypeFeatures(HarfBuzzTextParagraph self,
                                         const char **features,
                                         SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

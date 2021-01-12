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

#ifndef CAGE_BASIC_TEXT_PARAGRAPH_
#define CAGE_BASIC_TEXT_PARAGRAPH_

/**
 * @file SvBasicTextParagraph.h Basic text paragraph class API
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
#include <CAGE/Text/SvAbstractTextParagraph.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvBasicTextParagraph Simple text paragraph class
 * @ingroup CAGE_TextRendering
 * @{
 *
 * SvBasicTextParagraph is a very simple implementation of
 * the @ref SvTextParagraph, supporting only basic text formatting.
 *
 * @image html SvBasicTextParagraph.png
 **/


/**
 * Basic text paragraph class.
 * @class SvBasicTextParagraph
 * @extends SvAbstractTextParagraph
 **/
typedef struct SvBasicTextParagraph_ *SvBasicTextParagraph;


/**
 * Return the runtime type identification object
 * of the SvBasicTextParagraph class.
 *
 * @return basic text paragraph class
 **/
extern SvType
SvBasicTextParagraph_getType(void);

/**
 * Create new basic text paragraph.
 *
 * This is a convenience method that combines object allocation
 * and initialization.
 *
 * @memberof SvBasicTextParagraph
 *
 * @param[in] text      contents of the paragraph as UTF-8 string
 * @param[in] length    length of @a text, pass @c -1 if unknown
 * @param[in] font      handle to a font to be used for @a text
 * @param[in] fontSize  size (in pixels) of font for @a text
 * @param[in] color     color to render @a text in
 * @param[out] errorOut error info
 * @return              new basic text paragraph, @c NULL in case of error
 **/
extern SvBasicTextParagraph
SvBasicTextParagraphCreate(const char *text,
                           ssize_t length,
                           SvFont font,
                           unsigned int fontSize,
                           SvColor color,
                           SvErrorInfo *errorOut);

/**
 * Reset internal state.
 *
 * This method is an alias for SvAbstractTextParagraphReset().
 *
 * @memberof SvBasicTextParagraph
 *
 * @param[in] self      basic text paragraph handle
 * @param[out] errorOut error info
 **/
static inline void
SvBasicTextParagraphReset(SvBasicTextParagraph self,
                          SvErrorInfo *errorOut)
{
    SvAbstractTextParagraphReset((SvAbstractTextParagraph) self, errorOut);
}

/**
 * Set contents of the text paragraph.
 *
 * This method is an alias for SvAbstractTextParagraphSetText().
 *
 * @memberof SvBasicTextParagraph
 *
 * @param[in] self      basic text paragraph handle
 * @param[in] text      new contents of the paragraph as UTF-8 string
 * @param[in] length    length of @a text, pass @c -1 if unknown
 * @param[out] errorOut error info
 **/
static inline void
SvBasicTextParagraphSetText(SvBasicTextParagraph self,
                            const char *text,
                            ssize_t length,
                            SvErrorInfo *errorOut)
{
    SvAbstractTextParagraphSetText((SvAbstractTextParagraph) self, text, length, errorOut);
}

/**
 * Set text rendering attributes.
 *
 * This method is an alias for SvAbstractTextParagraphSetTextAttributes().
 *
 * @memberof SvBasicTextParagraph
 *
 * @param[in] self      basic text paragraph handle
 * @param[in] font      handle to a font to be used for rendering
 * @param[in] fontSize  font size in pixels
 * @param[in] color     color to render text in
 * @param[in] tracking  tracking (a.k.a. letter spacing) in 26.6 fixed
 *                      point format, pass @c 0 for default value
 * @param[out] errorOut error info
 **/
static inline void
SvBasicTextParagraphSetTextAttributes(SvBasicTextParagraph self,
                                      SvFont font,
                                      unsigned int fontSize,
                                      SvColor color,
                                      int tracking,
                                      SvErrorInfo *errorOut)
{
    SvAbstractTextParagraphSetTextAttributes((SvAbstractTextParagraph) self,
                                             font, fontSize, color, tracking,
                                             errorOut);
}

/**
 * Limit dimensions of a text box containing rendered text paragraph.
 *
 * This method is an alias for SvAbstractTextParagraphSetLimits().
 *
 * @memberof SvBasicTextParagraph
 *
 * @param[in] self      basic text paragraph handle
 * @param[in] maxWidth  max width of text box in pixels, @c 0 for no limit
 * @param[in] maxLines  max number of text lines in a box, @c 0 for no limit
 * @param[out] errorOut error info
 **/
static inline void
SvBasicTextParagraphSetLimits(SvBasicTextParagraph self,
                              unsigned int maxWidth,
                              unsigned int maxLines,
                              SvErrorInfo *errorOut)
{
    SvAbstractTextParagraphSetLimits((SvAbstractTextParagraph) self,
                                     maxWidth, maxLines,
                                     errorOut);
}

/**
 * Set policy of handling long unbreakable lines.
 *
 * This method is an alias for SvAbstractTextParagraphSetLineBreakPolicy().
 *
 * @memberof SvBasicTextParagraph
 *
 * @param[in] self      basic text paragraph handle
 * @param[in] policy    line break policy
 * @param[out] errorOut error info
 **/
static inline void
SvBasicTextParagraphSetLineBreakPolicy(SvBasicTextParagraph self,
                                       SvAbstractTextParagraphLineBreakPolicy policy,
                                       SvErrorInfo *errorOut)
{
    SvAbstractTextParagraphSetLineBreakPolicy((SvAbstractTextParagraph) self, policy, errorOut);
}

/**
 * Set marker that will be added after last rendered word to signal that part
 * of the text was omitted.
 *
 * This method is an alias for SvAbstractTextParagraphSetBreakMarker().
 *
 * @memberof SvBasicTextParagraph
 *
 * @param[in] self      basic text paragraph handle
 * @param[in] marker    marker text, @c NULL to disable this functionality
 * @param[in] enableOnEOL @c true to use marker when line ends with explicit
 *                      line break (LF character), @c false to use marker only
 *                      on automatically breaked lines
 * @param[out] errorOut error info
 **/
static inline void
SvBasicTextParagraphSetBreakMarker(SvBasicTextParagraph self,
                                   const char *marker,
                                   bool enableOnEOL,
                                   SvErrorInfo *errorOut)
{
    SvAbstractTextParagraphSetBreakMarker((SvAbstractTextParagraph) self,
                                          marker, enableOnEOL,
                                          errorOut);
}

/// @cond
typedef SvAbstractTextParagraphStats SvBasicTextParagraphStats;
/// @endcond

/**
 * Get information about text returned to the renderer.
 *
 * This method is an alias for SvAbstractTextParagraphGetStats().
 *
 * @memberof SvBasicTextParagraph
 *
 * @param[in] self      basic text paragraph handle
 * @param[out] stats    text paragraph statistics
 * @param[out] errorOut error info
 **/
static inline void
SvBasicTextParagraphGetStats(SvBasicTextParagraph self,
                             SvAbstractTextParagraphStats *stats,
                             SvErrorInfo *errorOut)
{
    SvAbstractTextParagraphGetStats((SvAbstractTextParagraph) self, stats, errorOut);
}

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

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

#ifndef CAGE_ABSTRACT_TEXT_PARAGRAPH_
#define CAGE_ABSTRACT_TEXT_PARAGRAPH_

/**
 * @file SvAbstractTextParagraph.h Abstract text paragraph class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvFont.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvAbstractTextParagraph Abstract text paragraph class
 * @ingroup CAGE_TextRendering
 * @{
 *
 * SvAbstractTextParagraph is a base class that can be extended to implement
 * the @ref SvTextParagraph.
 **/

/**
 * Line break policy.
 **/
typedef enum {
    /// signal error and break layout process
    SvAbstractTextParagraph_fail = 0,
    /// break at any position in the text
    SvAbstractTextParagraph_forceLineBreak,
    /// don't break, accept lines longer than max width
    SvAbstractTextParagraph_allowLongerLines,
} SvAbstractTextParagraphLineBreakPolicy;


/**
 * Text paragraph statistics.
 **/
typedef struct {
    /// total number of characters in the paragraph
    unsigned int charsCnt;
    /// number of characters returned to the renderer
    unsigned int returnedCharsCnt;
    /// number of characters skipped
    unsigned skippedCharsCnt;
    /// number of soft line breaks returned to the renderer
    unsigned softLineBreaksCnt;
    /// number of hard line breaks returned to the renderer
    unsigned hardLineBreaksCnt;
    /// @c true if line break marker was returned to the renderer
    bool lineBreakMarkerEmitted;
} SvAbstractTextParagraphStats;


/**
 * Abstract text paragraph class.
 * @class SvAbstractTextParagraph
 * @extends SvObject
 **/
typedef struct SvAbstractTextParagraph_ {
    /// super class
    struct SvObject_ super_;

    /// font used for rendering
    SvFont font;
    /// font size in pixels
    unsigned int fontSize;
    /// font aspect
    float fontAspect;
    /// color to render text in
    SvColor color;
    /// tracking (a.k.a. letter spacing) in 26.6 fixed point format
    int tracking;

    /// max width of text box in pixels, @c 0 for no limit
    unsigned int maxWidth;
    /// max number of text lines in a box, @c 0 for no limit
    unsigned int maxLines;

    /// line break policy
    SvAbstractTextParagraphLineBreakPolicy lineBreakPolicy;

    /// break marker character codes
    uint32_t *breakMarker;
    /// number of characters in @a breakMarker
    unsigned int breakMarkerLength;
    /**
     * @c true to use marker when line ends with explicit line break
     * (LF character), @c false to use marker only on automatically breaked lines
     **/
    bool breakMarkerOnEOL;
} *SvAbstractTextParagraph;

/**
 * Abstract text paragraph virtual methods table.
 **/
typedef const struct SvAbstractTextParagraphVTable_ {
    /// super class virtual methods table
    struct SvObjectVTable_ super_;

    /**
     * Initialize text paragraph instance.
     *
     * @param[in] self      abstract text paragraph handle
     * @param[in] text      contents of the paragraph as UTF-8 string
     * @param[in] length    length of @a text, pass @c -1 if unknown
     * @param[in] font      handle to a font to be used for @a text
     * @param[in] fontSize  size (in pixels) of font for @a text
     * @param[in] color     color to render @a text in
     * @param[out] errorOut error info
     * @return              @a self or @c NULL in case of error
     **/
    SvAbstractTextParagraph (*init)(SvAbstractTextParagraph self,
                                    const char *text,
                                    ssize_t length,
                                    SvFont font,
                                    unsigned int fontSize,
                                    SvColor color,
                                    SvErrorInfo *errorOut);

    /**
     * Reset internal state.
     *
     * @param[in] self      abstract text paragraph handle
     * @param[out] errorOut error info
     **/
    void (*reset)(SvAbstractTextParagraph self,
                  SvErrorInfo *errorOut);

    /**
     * Set contents of the text paragraph.
     *
     * @param[in] self      abstract text paragraph handle
     * @param[in] text      new contents of the paragraph as UTF-8 string
     * @param[in] length    length of @a text, pass @c -1 if unknown
     * @param[out] errorOut error info
     **/
    void (*setText)(SvAbstractTextParagraph self,
                    const char *text,
                    ssize_t length,
                    SvErrorInfo *errorOut);

    /**
     * Set text rendering attributes.
     *
     * @param[in] self      abstract text paragraph handle
     * @param[in] font      handle to a font to be used for rendering
     * @param[in] fontSize  font size in pixels
     * @param[in] color     color to render text in
     * @param[in] tracking  tracking (a.k.a. letter spacing) in 26.6 fixed
     *                      point format, pass @c 0 for default value
     * @param[out] errorOut error info
     **/
    void (*setTextAttributes)(SvAbstractTextParagraph self,
                              SvFont font,
                              unsigned int fontSize,
                              SvColor color,
                              int tracking,
                              SvErrorInfo *errorOut);

    /**
     * Set marker that will be added after last rendered word to signal that part
     * of the text was omitted.
     *
     * @param[in] self      abstract text paragraph handle
     * @param[in] marker    marker text, @c NULL to disable this functionality
     * @param[in] enableOnEOL @c true to use marker when line ends with explicit
     *                      line break (LF character), @c false to use marker only
     *                      on automatically breaked lines
     * @param[out] errorOut error info
     **/
    void (*setBreakMarker)(SvAbstractTextParagraph self,
                           const char *marker,
                           bool enableOnEOL,
                           SvErrorInfo *errorOut);

    /**
     * Get information about text returned to the renderer.
     *
     * @param[in] self      abstract text paragraph handle
     * @param[out] stats    text paragraph statistics
     * @param[out] errorOut error info
     **/
    void (*getStats)(SvAbstractTextParagraph self,
                     SvAbstractTextParagraphStats *stats,
                     SvErrorInfo *errorOut);
} *SvAbstractTextParagraphVTable;


/**
 * Return the runtime type identification object
 * of the SvAbstractTextParagraph class.
 *
 * @return abstract text paragraph class
 **/
extern SvType
SvAbstractTextParagraph_getType(void);

/**
 * Initialize text paragraph instance.
 *
 * @memberof SvAbstractTextParagraph
 *
 * @param[in] self      abstract text paragraph handle
 * @param[in] text      contents of the paragraph as UTF-8 string
 * @param[in] length    length of @a text, pass @c -1 if unknown
 * @param[in] font      handle to a font to be used for @a text
 * @param[in] fontSize  size (in pixels) of font for @a text
 * @param[in] color     color to render @a text in
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
static inline SvAbstractTextParagraph
SvAbstractTextParagraphInit(SvAbstractTextParagraph self,
                            const char *text,
                            ssize_t length,
                            SvFont font,
                            unsigned int fontSize,
                            SvColor color,
                            SvErrorInfo *errorOut)
{
    return SvInvokeVirtual(SvAbstractTextParagraph, self,
                           init,
                           text, length, font, fontSize, color,
                           errorOut);
}

/**
 * Reset internal state.
 *
 * This method prepares text paragraph instance to be used by text layout
 * engine. There is no need to call this method before text paragraph
 * is used for the first time (i.e. after creating it),
 * but it must be used before each consecutive use.
 *
 * @memberof SvAbstractTextParagraph
 *
 * @param[in] self      abstract text paragraph handle
 * @param[out] errorOut error info
 **/
static inline void
SvAbstractTextParagraphReset(SvAbstractTextParagraph self,
                             SvErrorInfo *errorOut)
{
    SvInvokeVirtual(SvAbstractTextParagraph, self, reset, errorOut);
}

/**
 * Set contents of the text paragraph.
 *
 * This method replaces previous content of the text paragraph
 * with new one. It also performs SvAbstractTextParagraphReset() internally,
 * so that paragraph instance can be re-used for text layout.
 *
 * @memberof SvAbstractTextParagraph
 *
 * @param[in] self      abstract text paragraph handle
 * @param[in] text      new contents of the paragraph as UTF-8 string
 * @param[in] length    length of @a text, pass @c -1 if unknown
 * @param[out] errorOut error info
 **/
static inline void
SvAbstractTextParagraphSetText(SvAbstractTextParagraph self,
                               const char *text,
                               ssize_t length,
                               SvErrorInfo *errorOut)
{
    SvInvokeVirtual(SvAbstractTextParagraph, self,
                    setText, text, length, errorOut);
}

/**
 * Set text rendering attributes.
 *
 * @memberof SvAbstractTextParagraph
 *
 * @param[in] self      abstract text paragraph handle
 * @param[in] font      handle to a font to be used for rendering
 * @param[in] fontSize  font size in pixels
 * @param[in] color     color to render text in
 * @param[in] tracking  tracking (a.k.a. letter spacing) in 26.6 fixed
 *                      point format, pass @c 0 for default value
 * @param[out] errorOut error info
 **/
static inline void
SvAbstractTextParagraphSetTextAttributes(SvAbstractTextParagraph self,
                                         SvFont font,
                                         unsigned int fontSize,
                                         SvColor color,
                                         int tracking,
                                         SvErrorInfo *errorOut)
{
    SvInvokeVirtual(SvAbstractTextParagraph, self,
                    setTextAttributes,
                    font, fontSize, color, tracking, errorOut);
}

/**
 * Limit dimensions of a text box containing rendered text paragraph.
 *
 * @memberof SvAbstractTextParagraph
 *
 * @param[in] self      abstract text paragraph handle
 * @param[in] maxWidth  max width of text box in pixels, @c 0 for no limit
 * @param[in] maxLines  max number of text lines in a box, @c 0 for no limit
 * @param[out] errorOut error info
 **/
extern void
SvAbstractTextParagraphSetLimits(SvAbstractTextParagraph self,
                                 unsigned int maxWidth,
                                 unsigned int maxLines,
                                 SvErrorInfo *errorOut);

/**
 * Set policy of handling long unbreakable lines.
 *
 * @memberof SvAbstractTextParagraph
 *
 * @param[in] self      abstract text paragraph handle
 * @param[in] policy    line break policy
 * @param[out] errorOut error info
 **/
extern void
SvAbstractTextParagraphSetLineBreakPolicy(SvAbstractTextParagraph self,
                                          SvAbstractTextParagraphLineBreakPolicy policy,
                                          SvErrorInfo *errorOut);

/**
 * Set marker that will be added after last rendered word to signal that part
 * of the text was omitted.
 *
 * @memberof SvAbstractTextParagraph
 *
 * @param[in] self      abstract text paragraph handle
 * @param[in] marker    marker text, @c NULL to disable this functionality
 * @param[in] enableOnEOL @c true to use marker when line ends with explicit
 *                      line break (LF character), @c false to use marker only
 *                      on automatically breaked lines
 * @param[out] errorOut error info
 **/
static inline void
SvAbstractTextParagraphSetBreakMarker(SvAbstractTextParagraph self,
                                      const char *marker,
                                      bool enableOnEOL,
                                      SvErrorInfo *errorOut)
{
    SvInvokeVirtual(SvAbstractTextParagraph, self,
                    setBreakMarker,
                    marker, enableOnEOL, errorOut);
}

/**
 * Get information about text returned to the renderer.
 *
 * @memberof SvAbstractTextParagraph
 *
 * @param[in] self      abstract text paragraph handle
 * @param[out] stats    text paragraph statistics
 * @param[out] errorOut error info
 **/
static inline void
SvAbstractTextParagraphGetStats(SvAbstractTextParagraph self,
                                SvAbstractTextParagraphStats *stats,
                                SvErrorInfo *errorOut)
{
    SvInvokeVirtual(SvAbstractTextParagraph, self,
                    getStats, stats, errorOut);
}

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

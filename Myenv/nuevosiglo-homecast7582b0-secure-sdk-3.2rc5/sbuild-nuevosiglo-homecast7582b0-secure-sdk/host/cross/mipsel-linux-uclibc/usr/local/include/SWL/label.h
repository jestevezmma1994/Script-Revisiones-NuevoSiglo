/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2007 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SWL_LABEL_H_
#define SWL_LABEL_H_

/**
 * @file SMP/SWL/swl/label.h Label widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvFont.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup SvLabel Label widget
 * @ingroup SWL_widgets
 * @{
 *
 * Label widget displays one or more lines of text.
 **/

/**
 * Type of text aligment used in label.
 **/
typedef enum {
    SvLabelAlignment_LEFT = 0, ///< align text to LEFT
    SvLabelAlignment_CENTER,   ///< align text to CENTER
    SvLabelAlignment_RIGHT,    ///< align text to RIGHT
    SvLabelAlignment_TOP,      ///< align text to TOP
    SvLabelAlignment_MIDDLE,   ///< align text to MIDDLE
    SvLabelAlignment_BOTTOM    ///< align text to BOTTOM
} SvLabelAlignment;

/**
 * This type groups parameters for creating a label widget.
 **/
struct SvLabelParams {
    SvApplication app;        ///< CUIT application handle
    SvWidget parent;          ///< parent widget handle
    int x;                    ///< x offset
    int y;                    ///< y offset
    int width;                ///< width of the newly created widget
    int height;               ///< height of the newly created widget

    SvFont font;              ///< font to be used to render normal text
    SvFont obliqueFont;       ///< font to be used to render oblique text
    SvFont boldFont;          ///< font to be used to render bold text
    SvFont boldObliqueFont;   ///< font to be used to render bold oblique text

    int fontSize;             ///< size of the font
    SvColor fontColor;        ///< color of the text
    SvLabelAlignment align;   ///< horizontal text alignment

    const char *value;        ///< initial text value of the label
    bool isMarkup;            ///< @c true if @a value is markup text
    bool isMultiline;         ///< @c true if is multiline text
    int maxLines;             ///< max lines count (default: no limit)
};

/** @cond NEVER */
#define LabelConstructor SvLabelParams
/** @endcond */


/**
 * Create label widget basing onto the given parameters.
 *
 * @param[in] params label construction parameters
 * @return           newly created widget or @c NULL in case of error
 **/
extern SvWidget
svLabelNewWithCaption(const struct SvLabelParams *params);

/**
 * Create label widget basing onto the given parameters, with width
 * and height automatically computed to fit the rendered text.
 *
 * @param[in] params label construction parameters
 * @return           newly created widget or @c NULL in case of error
 **/
extern SvWidget
svLabelAutoSize(const struct SvLabelParams *params);

/**
 * Create label widget using settings from the Settings Manager.
 *
 * This method creates label widget. Appearance of this widget is
 * controlled by the Settings Manager. It will use following parameters:
 *
 *  - width, height : integer, required,
 *  - font : font, required,
 *  - fontSize : integer, required,
 *  - textColor : color, required.
 *  - textAlign : left, right or center (string, optional, default: center),
 *  - vertAlign : top, middle or bottom (string, optional, default: middle),
 *  - multiLine : boolean, optional (default: false),
 *  - maxLines : integer, optional (default: no limit),
 *  - breakMarker : optional (default: none), requires multiLine and maxLines to be set,
 *  - text : initial contents (string, optional).
 *
 * @param[in] app             CUIT application handle
 * @param[in] widgetName      widget name
 * @return                    newly created widget or @c NULL in case of error
 **/
extern SvWidget
svLabelNewFromSM(SvApplication app, const char *widgetName);

/**
 * Create label widget using ratio settings.
 *
 * This method creates label widget. Appearance of this widget is
 * controlled by the Ratio. It will use following parameters:
 *
 *  - width, height : integer, required,
 *  - font : font, required,
 *  - fontSize : integer, required,
 *  - textColor : color, required.
 *  - textAlign : left, right or center (string, optional, default: center),
 *  - vertAlign : top, middle or bottom (string, optional, default: middle),
 *  - multiLine : boolean, optional (default: false),
 *  - maxLines : integer, optional (default: no limit),
 *  - breakMarker : optional (default: none), requires multiLine and maxLines to be set,
 *  - text : initial contents (string, optional).
 *
 * @param[in] app             CUIT application handle
 * @param[in] widgetName      widget name
 * @param[in] parentWidth     parent width
 * @param[in] parentHeight    parent height
 * @return                    newly created widget or @c NULL in case of error
 **/
extern SvWidget
svLabelNewFromRatio(SvApplication app, const char *widgetName, unsigned int parentWidth, unsigned int parentHeight);

/**
 * Get width of the label's text.
 *
 * @param[in] label           label widget handle
 * @return                    width of the widget's rendered text
 **/
extern int
svLabelGetWidth(SvWidget label);

/**
 * Get height of the label's text.
 *
 * @param[in] label           label widget handle
 * @return                    width of the widget's rendered text
 **/
extern int
svLabelGetHeight(SvWidget label);

/**
 * Set label contents.
 *
 * @param[in] label          label widget handle
 * @param[in] text           text to be displayed
 * @return                   @c 0 in case of an error, otherwise any other value
 **/
extern int
svLabelSetText(SvWidget label, const char *text);

/**
 * Set label color.
 *
 * @param[in] label          label widget handle
 * @param[in] color          new color of the text
 * @param[in] duration       duration of the color change effect
 * @return                   @c 0 in case of an error, otherwise any other value
 **/
extern int
svLabelSetTextColor(SvWidget label, SvColor color, double duration);

/**
 * Backwards compatibility function, calls svLabelSetTextColor().
 * @param[in] label          label widget handle
 * @param[in] color          new color of the text
 * @return                   @c 0 in case of an error, otherwise any other value
 **/
static inline int
svLabelSetColor(SvWidget label, SvColor color)
{
    return svLabelSetTextColor(label, color, 0.0);
}

/**
 * Set label contents from markup.
 *
 * @param[in] label          label widget handle
 * @param[in] markup         markup text to be displayed
 * @return                   @c 0 in case of an error, otherwise any other value
 **/
extern int
svLabelSetMarkupText(SvWidget label, const char *markup);

/**
 * Set label's font size.
 *
 * @param[in] label          label widget handle
 * @param[in] fontSize       fontSize
 * @return                   @c 0 in case of an error, otherwise any other value
 **/
extern int
svLabelSetFontSize(SvWidget label, int fontSize);

/**
 * Get the label font size
 *
 * @param[in] label          label widget handle
 * @return                   font size
 **/
extern int
svLabelGetFontSize(SvWidget label);

/**
 * @}
 **/

#endif

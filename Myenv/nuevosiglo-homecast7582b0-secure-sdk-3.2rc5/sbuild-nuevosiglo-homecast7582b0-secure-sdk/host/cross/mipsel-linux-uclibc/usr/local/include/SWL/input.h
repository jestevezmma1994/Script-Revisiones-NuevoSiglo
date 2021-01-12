/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
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

#ifndef SWL_INPUT_H_
#define SWL_INPUT_H_

/**
 * @file SMP/SWL/swl/input.h Input widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>

#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvFont.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/QBUIEvent.h>

/**
 * @defgroup SvInput Input widget
 * @ingroup SWL_widgets
 * @{
 *
 * Input widget allows to enter text using remote controller or keyboard.
 **/


/**
 * @defgroup SvInputBaseEvent Input widget base event class
 * @{
 **/

/**
 * Input widget base event class.
 * @class SvInputBaseEvent input.h <SWL/input.h>
 * @extends QBUIEvent
 **/
typedef struct SvInputBaseEvent_ *SvInputBaseEvent;

/**
 * SvInputBaseEvent class internals.
 **/
struct SvInputBaseEvent_ {
    /// super class
    struct QBUIEvent_ super_;
    /// current contents of the input widget
    char *contents;
};

/**
 * Get runtime type identification object representing SvInputBaseEvent class.
 *
 * @return Input widget base event class
 **/
extern SvType
SvInputBaseEvent_getType(void);

/**
 * @}
 **/


/**
 * @defgroup SvInputChangedEvent Input widget changed event class
 *
 * This kind of event is posted every time the contents of the input
 * widget is changed.
 *
 * @{
 **/

/**
 * Input widget changed event class.
 * @class SvInputChangedEvent input.h <SWL/input.h>
 * @extends SvInputBaseEvent
 **/
typedef struct SvInputChangedEvent_ *SvInputChangedEvent;

/**
 * SvInputChangedEvent class internals.
 **/
struct SvInputChangedEvent_ {
    /// super class
    struct SvInputBaseEvent_ super_;
};

/**
 * Get runtime type identification object representing SvInputChangedEvent class.
 *
 * @return Input widget changed event class
 **/
extern SvType
SvInputChangedEvent_getType(void);

/**
 * @}
 **/


/**
 * @defgroup SvInputSelectedEvent Input widget selected event class
 *
 * This kind of event is posted when input widget is clicked
 * (OK key is pressed.)
 *
 * @{
 **/

/**
 * Input widget selected event class.
 * @class SvInputSelectedEvent input.h <SWL/input.h>
 * @extends SvInputBaseEvent
 **/
typedef struct SvInputSelectedEvent_ *SvInputSelectedEvent;

/**
 * SvInputSelectedEvent class internals.
 **/
struct SvInputSelectedEvent_ {
    /// super class
    struct SvInputBaseEvent_ super_;
};

/**
 * Get runtime type identification object representing SvInputSelectedEvent class.
 *
 * @return Input widget selected event class
 **/
extern SvType
SvInputSelectedEvent_getType(void);

/**
 * @}
 **/


/**
 * Mode of input widget.
 **/
typedef enum {
    SvInputMode_unknown = 0,    ///< unknown mode
    SvInputMode_text,           ///< for entering/editing text
    SvInputMode_numeric,        ///< for entering/editing numbers
    SvInputMode_IPv4,           ///< for entering/editing IPv4 address
} SvInputMode;

/**
 * Cursor visibility modes.
 **/
typedef enum {
    /// text cursor is never visible
    SvInputCursorVisibility_hidden = -1,
    /// text cursor is visible only when widget has focus (default)
    SvInputCursorVisibility_normal = 0,
    /// text cursor is always visible
    SvInputCursorVisibility_always
} SvInputCursorVisibility;


/**
 * SvInputParams parameters.
 * Struct used for SvInput with given params
 **/
typedef struct SvInputParams_ {
    int width;                  ///< input width
    int height;                 ///< input height
    int horizPadding;           ///< horizontal padding
    int vertPadding;            ///< vertical padding
    int fontSize;               ///< font size

    SvColor textColor;          ///< text color
    SvColor inactiveTextColor;  ///< inactive text color
    SvColor disabledTextColor;  ///< disabled text color
    SvBitmap bg;                ///< background
    SvBitmap inactiveBg;        ///< inactive background
    SvBitmap disabledBg;        ///< disabled background
    SvBitmap textCursor;        ///< text cursor
    SvBitmap blockCursor;       ///< block cursor
    SvBitmap leftArrow;         ///< left arrow
    SvBitmap rightArrow;        ///< right arrow

    SvFont font;                ///< font
    SvInputMode mode;           ///< mode
    int maxChars;               ///< maximal number of input characters
    double cursorBlinkPeriod;   ///< cursor blink period
} SvInputParams;

/**
 * Create input widget using params.
 *
 * This function creates input widget from given params
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @param[in] params       parameters for new input widget
 * @return                 created widget, @c NULL in case of error
 */
SvWidget
SvInputNewFromParams(SvApplication app,
                     const char *widgetName,
                     SvInputParams params);

/**
 * Create input widget using settings from the Settings Manager.
 *
 * This function creates input widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 *  - width, height : integer, required,
 *  - padding, horizPadding, vertPadding : integer, all optional (default: 0),
 *  - mode : string, required if @a mode is unknown ('text' or 'numeric'),
 *  - font : font, required,
 *  - fontSize : integer, required,
 *  - textColor : color, required,
 *  - inactiveTextColor : color, optional (default: textColor),
 *  - disabledTextColor : color, optional (default: inactiveTextColor),
 *  - bg, inactiveBg, disabledBg: bitmap, optional,
 *  - leftArrow, rightArrow: bitmap, optional (default: none),
 *  - textCursor: bitmap, optional (default: orange vertical line),
 *  - blockCursor: bitmap, optional (default: orange underscore),
 *  - cursorBlinkPeriod: double, optional (default: no blinking).
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @param[in] mode         input widget mode, pass SvInputMode::SvInputMode_unknown
 *                         to get it from settings
 * @param[in] maxChars     maximum number of characters
 * @return                 created widget, @c NULL in case of error
 */
extern SvWidget
svInputNew(SvApplication app, const char *widgetName,
           SvInputMode mode, int maxChars);

/**
 * Create input widget using ratio settings from the Settings Manager.
 *
 * This function creates input widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 *  - widthRatio, heightRatio : double, required,
 *  - paddingRatio, horizPaddingRatio, vertPaddingRatio : double, all optional (default: 0),
 *  - mode : string, required if @a mode is unknown ('text' or 'numeric'),
 *  - font : font, required,
 *  - fontSize : double, required,
 *  - textColor : color, required,
 *  - inactiveTextColor : color, optional (default: textColor),
 *  - disabledTextColor : color, optional (default: inactiveTextColor),
 *  - bg, inactiveBg, disabledBg: bitmap, optional,
 *  - leftArrow, rightArrow: bitmap, optional (default: none),
 *  - textCursor: bitmap, optional (default: orange vertical line),
 *  - blockCursor: bitmap, optional (default: orange underscore),
 *  - cursorBlinkPeriod: double, optional (default: no blinking).
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @param[in] parentWidth   parent width
 * @param[in] parentHeight  parent height
 * @param[in] mode         input widget mode, pass SvInputMode::SvInputMode_unknown
 *                         to get it from settings
 * @param[in] maxChars     maximum number of characters
 * @return                 created widget, @c NULL in case of error
 */
extern SvWidget
svInputNewFromRatio(SvApplication app, const char *widgetName,
                    unsigned int parentWidth, unsigned int parentHeight,
                    SvInputMode mode, int maxChars);

/**
 * Backwards compatibility function, calls svInputNew()
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @param[in] mode         input widget mode, pass SvInputMode::SvInputMode_unknown
 *                         to get it from settings
 * @param[in] maxChars     maximum number of characters
 * @return                 created widget, @c NULL in case of error
 */
static inline SvWidget
svInputNewFromSM(SvApplication app, const char *widgetName,
                 SvInputMode mode, int maxChars)
{
    return svInputNew(app, widgetName, mode, maxChars);
}

/**
 * Setup background bitmaps for input widget.
 *
 * This function changes background bitmaps of input widget.
 * You can pass two different bitmaps: @a unfocusedBmp will be used when
 * widget is not focused, otherwise @a focusedBmp will be used.
 * Both bitmaps are optional.
 *
 * @param[in] w               input widget handle
 * @param[in] unfocusedBmp    background bitmap for unfocused widget
 * @param[in] focusedBmp      background bitmap for focused widget
 * @param[in] disabledBmp     background bitmap for disabled widget
 */
extern void
svInputSetBG(SvWidget w, SvBitmap unfocusedBmp, SvBitmap focusedBmp, SvBitmap disabledBmp);

/**
 * Set text color.
 *
 * @param[in] w            input widget handle
 * @param[in] activeColor  text color for focused widget
 * @param[in] inactiveColor text color for unfocused widget
 */
extern void
svInputSetTextColor(SvWidget w, SvColor activeColor, SvColor inactiveColor);

/**
 * Change input widget's mode.
 *
 * @param[in] w               input widget handle
 * @param[in] mode            input widget mode
 */
extern void
svInputSetMode(SvWidget w, SvInputMode mode);

/**
 * Enable / disable password mode (hiding the characters).
 *
 * @param[in] w               input widget handle
 * @param[in] passwordMode    if @c true, the password mode will be enabled
 * @param[in] maskChar        unicode mask character code (for example: L'*')
 */
extern void
svInputSetPasswordMode(SvWidget w, bool passwordMode, unsigned int maskChar);

/**
 * Set the input widget's contents.
 *
 * @param[in] w               input widget handle
 * @param[in] text            new content
 */
extern void
svInputSetText(SvWidget w, const char *text);

/**
 * Append given text to actual input widget's contents.
 *
 * @param[in] w               input widget handle
 * @param[in] str             text to be appended
 */
extern void
svInputAppend(SvWidget w, const char *str);

/**
 * Remove one character before cursor.
 *
 * @param[in] w               input widget handle
 */
extern void
svInputBackspace(SvWidget w);

/**
 * Clear the contents of the Input widget.
 *
 * @param[in] w               Input widget handle
 */
extern void
svInputClear(SvWidget w);

/**
 * Get copy of input widget's content.
 *
 * @param[in] w               input widget handle
 * @return                    pointer to the copy of text in the widget
 *                            (caller is responsible for freeing it)
 */
extern char *
svInputGetText(SvWidget w);

/**
 * Set cursor mode.
 *
 * @param[in] w               input widget handle
 * @param[in] visibility      cursor visibility
 */
extern void
svInputSetCursor(SvWidget w, SvInputCursorVisibility visibility);

/**
 * Enable or disable text editing.
 *
 * @param[in] w               input widget handle
 * @param[in] readOnly        if @c true, editing will be disabled
 */
extern void
svInputSetReadOnly(SvWidget w, bool readOnly);

/**
 * Set input map.
 *
 * @param[in] w               input widget handle
 * @param[in] keymap          string containing characters entered for each
 *                            input code from '0' to '9', separated with LFs
 * @return                    @c 0 in case of error, any other value otherwise
 */
extern int
svInputSetKeymap(SvWidget w, const char *keymap);

/**
 * Set disabled/enabled.
 *
 * @param[in] w               input widget handle
 * @param[in] isDisabled      if @c true widget is disabled afterwards, otherwise widget in enabled
 */
extern void
svInputSetDisabled(SvWidget w, bool isDisabled);


/**
 * @}
 **/

#endif

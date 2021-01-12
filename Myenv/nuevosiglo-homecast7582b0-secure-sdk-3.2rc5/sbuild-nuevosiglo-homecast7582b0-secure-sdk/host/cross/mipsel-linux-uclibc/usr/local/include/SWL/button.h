/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
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

#ifndef SWL_BUTTON_H_
#define SWL_BUTTON_H_

#include <stdbool.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvFont.h>
#include <CUIT/Core/types.h>

/**
 * @file SMP/SWL/swl/button.h Button widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

/**
 * @defgroup SvButton Button widget
 * @ingroup SWL_widgets
 * @{
 *
 * Button widget sends a notification when it is clicked.
 **/

/**
 * Type of UI event sent by button widget when it is pushed (clicked).
 * @class SvButtonPushedEvent
 * @extends QBUIEvent
 **/
typedef struct SvButtonPushedEvent_ *SvButtonPushedEvent;

/**
 * Get runtime type identification object representing button pushed event class.
 *
 * @relates SvButtonPushedEvent
 *
 * @return button pushed event class
 **/
extern SvType
SvButtonPushedEvent_getType(void);

/**
 * Create button widget.
 *
 * This function creates button widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 *  - padding : integer, optional (default: 0),
 *  - topPadding, bottomPadding : integer, optional (default: 0),
 *  - font : font, required,
 *  - fontSize : integer, required,
 *  - subfontSize : integer, optional (default: 0.85*fontSize),
 *  - vSpacing : integer, optional (default: -0.2*fontSize),
 *  - textColor : color, required,
 *  - inactiveTextColor : color, optional (default: textColor),
 *  - bg, inactiveBg: bitmap, required,
 *  - caption, markupCaption: string, optional,
 *  - icon: bitmap, optional (will be displayed together with the caption),
 *  - textAlign : left, right or center (string, optional, default: center),
 *  - duration : focus effect duration (double, optional, default: 0).
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @return                 created widget, @c NULL in case of error
 **/
extern SvWidget
svButtonNew(SvApplication app, const char *widgetName);

/**
 * Create button widget.
 *
 * This function is an extended version of svButtonNew().
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @param[in] caption      button caption
 * @param[in] tag          value that identifies the button
 * @param[in] target       ID of widget where notifications will be sent
 * @return                 created widget, @c NULL in case of error
 **/
extern SvWidget
svButtonNewFromSM(SvApplication app, const char *widgetName,
                  const char *caption, int tag, SvWidgetId target);

/**
 * Create button widget from ratio settings.
 *
 * This function is an extended version of svButtonNew().
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @param[in] caption      button caption
 * @param[in] tag          value that identifies the button
 * @param[in] target       ID of widget where notifications will be sent
 * @param[in] parentWidth  parent Width
 * @param[in] parentHeight parent Height
 * @return                 created widget, @c NULL in case of error
 **/
extern SvWidget
svButtonNewFromRatio(SvApplication app, const char *widgetName,
                     const char *caption, int tag, SvWidgetId target, unsigned int parentWidth, unsigned int parentHeight);

/**
 * Set notification target.
 *
 * This method sets target widget that will receive @c SV_EVENT_BUTTON_PUSHED
 * events.
 *
 * @param[in] w         button widget handle
 * @param[in] target    ID of the target widget
 * @return              @c 0 in case of an error, otherwise any other value
 */
extern int
svButtonSetNotificationTarget(SvWidget w, SvWidgetId target);

/**
 * Get caption of the button widget.
 *
 * @param[in] w         button widget handle
 * @return              caption of the button widget
 **/
extern const char *
svButtonGetCaption(SvWidget w);

/**
 * Set caption of the button widget.
 *
 * @param[in] w         button widget handle
 * @param[in] caption   new caption for the button widget
 * @return              @c 0 in case of an error, otherwise any other value
 **/
extern int
svButtonSetCaption(SvWidget w, const char *caption);

/**
 * Set caption of the button widget and resize the button to fit the content.
 *
 * @param[in] w         button widget handle
 * @param[in] caption   new subcaption for the button widget
 * @return              @c 0 in case of an error, otherwise any other value
 **/
extern int
svButtonSetCaptionAndResize(SvWidget w, const char *caption);

/**
 * Set subcaption of the button widget.
 *
 * @param[in] w             button widget handle
 * @param[in] subcaption    new subcaption for the button widget
 * @return                  @c 0 in case of an error, otherwise any other value
 **/
extern int
svButtonSetSubcaption(SvWidget w, const char *subcaption);

/**
 * Set subcaption of the button widget and resize the button to fit the content.
 *
 * @param[in] w             button widget handle
 * @param[in] subcaption    new subcaption for the button widget
 * @return                  @c 0 in case of an error, otherwise any other value
 **/
extern int
svButtonSetSubcaptionAndResize(SvWidget w, const char *subcaption);

/**
 * Set markup text as caption of the button widget.
 *
 * @param[in] w         button widget handle
 * @param[in] markup    markup text for caption of the button widget
 * @return              @c 0 in case of an error, otherwise any other value
 **/
extern int
svButtonSetMarkupCaption(SvWidget w, const char *markup);

/**
 * Change button state to disabled or enabled.
 *
 * @note Disabled button won't take focus and is displayed in a different way.
 *
 * @param[in] button          button widget handle
 * @param[in] isDisabled      if @c true widget is disabled afterwards, otherwise wigdet in enabled
 * @return                    @c 0 in case of an error, otherwise any other value
 */
extern int
svButtonSetDisabled(SvWidget button, bool isDisabled);

/**
 * Check if button is disabled.
 *
 * @param[in] button    button widget handle
 * @return              @c true if button is disabled
 **/
extern bool
svButtonIsDisabled(SvWidget button);

/**
 * Calculate button's required width
 *
 * @param[in] w    button widget handle
 * @return         @c required button's width
 */
extern int
SvButtonCalcWidth(SvWidget w);

/**
 * Sets button's width to new value
 *
 * @param[in] w         button widget handle
 * @param[in] width     new button's width
 */
extern void
SvButtonSetWidth(SvWidget w, int width);

/**
 * Get minimum button's width
 *
 * @param[in] w     button widget handle
 * @return          @c minimum button's width
 */
extern int
SvButtonGetMinWidth(SvWidget w);


/**
 * @}
 **/

#endif

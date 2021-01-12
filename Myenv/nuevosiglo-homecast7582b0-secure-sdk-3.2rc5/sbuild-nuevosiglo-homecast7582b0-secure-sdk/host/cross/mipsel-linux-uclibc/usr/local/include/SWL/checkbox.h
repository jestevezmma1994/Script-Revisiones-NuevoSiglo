/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SWL_CHECKBOX_H_
#define SWL_CHECKBOX_H_

/**
 * @file SMP/SWL/swl/checkbox.h
 * @brief Check box widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup SvCheckBox Check box widget
 * @ingroup SWL_widgets
 * @{
 *
 * Check box widget allows to toggle its state between active and inactive.
 **/

/**
 * Type of UI event sent by check box widget when its state changes.
 * @class SvCheckBoxToggledEvent
 * @extends QBUIEvent
 **/
typedef struct SvCheckBoxToggledEvent_ *SvCheckBoxToggledEvent;

/**
 * Get runtime type identification object representing check box toggled event class.
 *
 * @relates SvCheckBoxToggledEvent
 *
 * @return check box toggled event class
 **/
extern SvType
SvCheckBoxToggledEvent_getType(void);

/**
 * Create check box widget.
 *
 * This function creates check box widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 * - width, height - integer, required,
 * - tickMarkOn, tickMarkOff - bitmap, required,
 * - inactiveTickMarkOn, inactiveTickMarkOff - bitmap, optional,
 * - bg, inactiveBg - bitmap, optional,
 * - bgAlign - string {left, right, center}, optional
 * - leftPadding - integer, optional
 * - font - font, optional,
 * - fontSize - integer, optional,
 * - textColor, inactiveTextColor, disabledTextColor - color, optional,
 * - spacing - integer, optional,
 * - duration - double, optional,
 * - caption, markupCaption - string, optional,
 * - disabled - bool, optional.
 *
 * Check box can exist separately or in a group of check boxes,
 * in which only one of them can be checked (they work as radio buttons).
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    name of the widget used with Settings Manager
 * @param[in] radioGroup    handle to existing check box widget, which will connect newly created check box to a group,
 *                          (@c NULL for a stand-alone check box)
 * @param[in] caption       caption text (overrides caption from settings), can be @c NULL
 * @return                  newly created widget or @c NULL in case of error
 */
extern SvWidget
svCheckBoxNew(SvApplication app, const char *widgetName,
              SvWidget radioGroup, const char *caption);

/**
 * Set active/inactive.
 *
 * @param[in] checkBox        check box widget handle
 * @param[in] isActive        if @c true widget is checked afterwards, otherwise wigdet in unchecked
 * @return                    @c 0 in case of an error, otherwise any other value
 */
extern int
svCheckBoxSetActive(SvWidget checkBox, bool isActive);

/**
 * Set disabled/enabled.
 *
 * This method changes state of a check box widget between enabled and disabled.
 * Disabled check box won't take focus and is displayed in a different way.
 *
 * @param[in] checkBox        check box widget handle
 * @param[in] isDisabled      if @c true widget is disabled afterwards, otherwise wigdet in enabled
 * @return                    @c 0 in case of an error, otherwise any other value
 */
extern int
svCheckBoxSetDisabled(SvWidget checkBox, bool isDisabled);

/**
 * Check if check box widget is disabled.
 *
 * @param[in] checkBox        check box widget handle
 * @return                    @c true if widget is disabled, @c false otherwise
 */
extern bool
svCheckBoxIsDisabled(SvWidget checkBox);

/**
 * Check state of the check box widget
 *
 * @param[in] checkBox        check box widget handle
 * @return                    @c true if widget is checked, @c false otherwise
 */
extern bool
svCheckBoxIsActive(SvWidget checkBox);

/**
 * Get caption of the check box widget.
 *
 * @param[in] w         check box widget handle
 * @return              caption of the check box widget
 **/
extern const char *
svCheckBoxGetCaption(SvWidget w);

/**
 * Set caption of the check box widget.
 *
 * @param[in] w         check box widget handle
 * @param[in] caption   new caption for the check box widget
 * @return              @c 0 in case of an error, otherwise any other value
 **/
extern int
svCheckBoxSetCaption(SvWidget w, const char *caption);

/**
 * Set markup text as caption of the check box widget.
 *
 * @param[in] w         check box widget handle
 * @param[in] markup    markup text for caption of the check box widget
 * @return              @c 0 in case of an error, otherwise any other value
 **/
extern int
svCheckBoxSetMarkupCaption(SvWidget w, const char *markup);

/**
 * Configure standalone check box widget to make it act like a radio button.
 *
 * @param[in] checkBox      check box widget handle
 * @param[in] radioMode     @c true to make check box behave like a radio button
 * @return                  @c 0 in case of an error, otherwise any other value
 **/
extern int
svCheckBoxSetRadioMode(SvWidget checkBox, bool radioMode);


/**
 * @}
 **/

#endif

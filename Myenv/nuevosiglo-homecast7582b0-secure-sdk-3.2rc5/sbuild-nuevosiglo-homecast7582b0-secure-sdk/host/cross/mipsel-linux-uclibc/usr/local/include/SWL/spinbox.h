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

#ifndef SWL_SPINBOX_H_
#define SWL_SPINBOX_H_

/**
 * @file SMP/SWL/swl/spinbox.h Spin box widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/QBUIEvent.h>

/**
 * @defgroup SvSpinBox Spin box widget
 * @ingroup SWL_widgets
 * @{
 *
 * Spin box widget allows to choose one numeric value from a specified range.
 **/

/**
 * State of the spin box widget.
 **/
typedef struct {
    /// minimum value of the spin box
    int min;
    /// maximum value of the spin box
    int max;
    /// current value of the spin box
    int current;
} SvSpinBoxState;

/**
 * Type of UI event sent by spin box widget when its value changes.
 * @class SvSpinBoxUpdatedEvent
 * @extends QBUIEvent
 **/
typedef struct SvSpinBoxUpdatedEvent_ *SvSpinBoxUpdatedEvent;

/**
 * SvSpinBoxUpdatedEvent class internals.
 **/
struct SvSpinBoxUpdatedEvent_ {
    /// super class
    struct QBUIEvent_ super_;
    /// state after update
    SvSpinBoxState state;
};

/**
 * Get runtime type identification object representing spin box updated event class.
 *
 * @relates SvSpinBoxUpdatedEvent
 *
 * @return spin box updated event class
 **/
extern SvType
SvSpinBoxUpdatedEvent_getType(void);

/**
 * Create new spin box widget.
 *
 * Spin box is a widget that displays an integer from the range
 * [@a minValue, @a maxValue] and allows to change it by @a step up or down
 * using command keys (by default they are PGUP, PGDN, CHUP, CHDN).
 * Its appearance is controlled by the Settings Manager. It uses following
 * parameters:
 *
 * - width, height - integer, required,
 * - tickMarkOn, tickMarkOff - bitmap, required,
 * - inactiveTivkMarkOn, inactiveTickMarkOff - bitmap, optional,
 * - bg, inactiveBg, disabledBg - bitmap, optional,
 * - font - font, required,
 * - fontSize - integer, required,
 * - textColor, inactiveTextColor, disabledTextColor - color, required,
 * - leftPadding, rightPadding - integer, optional,
 * - minDigitsCount - integer, optional,
 * - textAlign - string (left, right or center), optional,
 * - spacing - integer, optional,
 * - duration - double, optional.
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    name of the widget used with Settings Manager
 * @param[in] minValue      lowest available value
 * @param[in] maxValue      highest available value
 * @param[in] step          interval of value change
 * @return                  newly created spin box widget, @c NULL in case of error
 */
extern SvWidget
svSpinBoxNew(SvApplication app, const char *widgetName,
             int minValue, int maxValue, int step);

/**
 * Set range of values accepted by spin box widget.
 *
 * @param[in] spinBox      spin box widget handle
 * @param[in] minValue      lowest available value
 * @param[in] maxValue      highest available value
 * @return                 @c 0 in case of an error, otherwise any other value
 **/
extern int
svSpinBoxSetBounds(SvWidget spinBox, int minValue, int maxValue);

/**
 * Get state (current value and range of accepted values)
 * of the spin box widget.
 *
 * @param[in] spinBox      spin box widget handle
 * @return                 current state
 **/
extern SvSpinBoxState
svSpinBoxGetState(SvWidget spinBox);

/**
 * Get current value of the spin box widget.
 *
 * @param[in] spinBox      spin box widget handle
 * @return                 current spin box value
 **/
extern int
svSpinBoxGetValue(SvWidget spinBox);

/**
 * Set value of the spin box widget.
 *
 * @param[in] spinBox       spin box widget handle
 * @param[in] newValue      desired value, must be beetwen widget's
 *                          minValue and maxValue
 * @return                  @c O if the parameter is correct, @c -1 otherwise
 **/
extern int
svSpinBoxSetValue(SvWidget spinBox, int newValue);

/**
 * Setup which keys are handled by spin box widget.
 *
 * @param[in] spinBox       spin box widget handle
 * @param[in] upKey         command key code for decreasing value of spin box
 * @param[in] downKey       command key code for increasing value of spin box
 * @return                  @c 0 in case of an error, otherwise any other value
 **/
extern int
svSpinBoxSetKeyCodes(SvWidget spinBox,
                     unsigned int upKey, unsigned int downKey);

/**
 * Set disabled/enabled state.
 *
 * This method enables or disables spin box widget.
 * Disabled spin box does not take focus and is displayed in a different way.
 *
 * @param[in] spinBox       spin box widget handle
 * @param[in] isDisabled    @c true to disable widget, @c false to enable
 * @return                  @c 0 in case of an error, otherwise any other value
 **/
extern int
svSpinBoxSetDisabled(SvWidget spinBox, bool isDisabled);


/**
 * @}
 **/

#endif

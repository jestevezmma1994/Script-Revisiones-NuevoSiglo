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

#ifndef QBCOMBOBOX_H_
#define QBCOMBOBOX_H_

/**
 * @file  QBComboBox.h
 * @brief ComboBox Class API
 */

#include <CUIT/Core/types.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvStringBuffer.h>
#include <SvFoundation/SvArray.h>
#include <stdbool.h>

/**
 * @defgroup QBComboBox ComboBox
 * @ingroup CubiTV_widgets
 * @{
 *
 * ComboBox is a widget that allows the user to select one value out of given
 * content list. Optionally it can display custom value not from list.
 *
 * A value can be any SvObject. It's converted to a string using
 * the 'prepare' callback.
 *
 * It displays a box with a value and arrows indicating availability of other
 * values.
 */

/**
 * @defgroup callbackInterfaces Callback interfaces
 * @{
 */

/**
 * Interface of the 'input' callback.
 * This callback is called to handle input events.
 *
 * @param[in]   target      callback target
 * @param[in]   combobox    calling combobox handle
 * @param[in]   e           input event
 * @return                  event handler status
 */
typedef bool (*QBComboBoxInputCallback)(void *target, SvWidget combobox, SvInputEvent e);

/**
 * Interface of the 'prepare' callback.
 * This callback is used to create a string representing given value.
 *
 * @param[in]   target      callback target
 * @param[in]   combobox    calling combobox handle
 * @param[in]   value       value
 * @return                  string representing value
 */
typedef SvString (*QBComboBoxPrepareDataCallback)(void *target, SvWidget combobox, SvObject value);

/**
 * Interface of the 'change' callback.
 * This callback is used to get the position of value in content that's next to the given custom one.
 *
 * @param[in]   target      callback target
 * @param[in]   combobox    calling combobox handle
 * @param[in]   value       custom value
 * @param[in]   key         pressed key @see QBInputCodes, or 0 if called from QBComboBoxSetContent.
 * @return                  index of value next to given custom one
 */
typedef int (*QBComboBoxChangeCallback)(void *target, SvWidget combobox, SvObject value, int key);

/**
 * Interface of the 'current' callback.
 * This callback is called when selected value is changed or when custom value is set.
 *
 * @param[in]   target      callback target
 * @param[in]   combobox    calling combobox handle
 * @param[in]   value       new combobox value
 */
typedef void (*QBComboBoxCurrentCallback)(void *target, SvWidget combobox, SvObject value);

/**
 * Interface of the 'start' callback.
 * This callback is called when the ComboBox gains focus.
 *
 * @param[in]   target          callback target
 * @param[in]   combobox        calling combobox handle
 * @param[in]   selectedValue   value under selected position
 */
typedef void (*QBComboBoxOnStartCallback) (void *target, SvWidget combobox, SvObject selectedValue);

/**
 * Interface of the 'finish' callback.
 * This callback is called when the ComboBox loses focus.
 *
 * @param[in]   target          callback target
 * @param[in]   combobox        calling combobox handle
 * @param[in]   selectedValue   value under selected position
 * @param[in]   userValue       custom value
 */
typedef void (*QBComboBoxOnFinishCallback)(void *target, SvWidget combobox, SvObject selectedValue, SvObject userValue);

/**
 * @}
 */

/**
 * Structure used to pass a set of pointers
 * to callbacks for QBComboBox.
 **/
struct QBComboBoxCallbacks_t {
    /**
     * 'prepare' callback or NULL
     **/
    QBComboBoxPrepareDataCallback prepare;
    /**
     * 'input' callback or NULL
     **/
    QBComboBoxInputCallback input;
    /**
     * 'change' callback or NULL
     **/
    QBComboBoxChangeCallback change;
    /**
     * 'current' callback or NULL
     **/
    QBComboBoxCurrentCallback current;
    /**
     * 'onStart' callback or NULL
     **/
    QBComboBoxOnStartCallback onStart;
    /**
     * 'onFinish' callback or NULL
     **/
    QBComboBoxOnFinishCallback onFinish;
};
typedef struct QBComboBoxCallbacks_t QBComboBoxCallbacks;

// For ComboBoxes caontainig large set of integer values, eg. frequency list
typedef struct QBComboBoxIntegerDataSource_ * QBComboBoxIntegerDataSource;
QBComboBoxIntegerDataSource QBComboBoxIntegerDataSourceCreate(SvErrorInfo *errorOut);
void QBComboBoxIntegerDataSourceSetLength(QBComboBoxIntegerDataSource self, size_t length);
void QBComboBoxIntegerDataSourceSetMultiplier(QBComboBoxIntegerDataSource self, size_t multiplier);
int QBComboBoxIntegerDataSourceIndexOfObject(QBComboBoxIntegerDataSource self, SvObject obj);

/**
 * Create new ComboBox.
 *
 * ComboBox is a generic widget with several subwidgets, which are created and
 * attached using settings manager. Indentation shows widget hierarchy.
 * - "widgetName" - svWidget - required
 *   - "widgetName.Active"   - QBFrame  - optional - is displayed when ComboBox is focused
 *   - "widgetName.Inactive" - QBFrame  - optional - is displayed when ComboBox is not focused
 *   - "widgetName.Box"      - svWidget - required
 *     - "widgetName.Label"  - svLabel  - required - used to display content
 *   - "widgetName.Arrow.Left", "widgetName.Arrow.Right", "widgetName.Arrow.Up", "widgetName.Arrow.Down"
 *                           - svWidget - optional
 *
 * Additional options:
 * - "widgetName"
 *   - loopPossible - boolean - optional, default: false
 * - "widgetName.Arrow.*"
 *   - "active"     - bitmap  - required - is displayed when next (for Down or Right) or previous (for Up or Left) value is available in content
 *   - "inactive"   - bitmap  - required - is displayed otherwise
 *
 * @param[in]   app         CUIT application handle
 * @param[in]   widgetName  Widget name in settings file
 * @return                  ComboBox widget handle
 */
SvWidget QBComboBoxNewFromSM(SvApplication app, const char *widgetName);

/**
 * Create new ComboBox From Ratio setttings.
 *
 * ComboBox is a generic widget with several subwidgets, which are created and
 * attached using settings manager. Indentation shows widget hierarchy.
 * - "widgetName" - svWidget - required
 *   - "widgetName.Active"   - QBFrame  - optional - is displayed when ComboBox is focused
 *   - "widgetName.Inactive" - QBFrame  - optional - is displayed when ComboBox is not focused
 *   - "widgetName.Box"      - svWidget - required
 *     - "widgetName.Label"  - svLabel  - required - used to display content
 *   - "widgetName.Arrow.Left", "widgetName.Arrow.Right", "widgetName.Arrow.Up", "widgetName.Arrow.Down"
 *                           - svWidget - optional
 *
 * Additional options:
 * - "widgetName"
 *   - loopPossible - boolean - optional, default: false
 * - "widgetName.Arrow.*"
 *   - "active"     - bitmap  - required - is displayed when next (for Down or Right) or previous (for Up or Left) value is available in content
 *   - "inactive"   - bitmap  - required - is displayed otherwise
 *
 * @param[in]   app             CUIT application handle
 * @param[in]   widgetName      Widget name in settings file
 * @param[in]   parentWidth     Parent Width
 * @param[in]   parentHeight    Parent Height
 * @return                      ComboBox widget handle
 */
SvWidget QBComboBoxNewFromRatio(SvApplication app, const char *widgetName, unsigned int parentWidth, unsigned int parentHeight);

/**
 * Get position of selected value.
 *
 * @param[in]   w   ComboBox widget handle
 * @return          selected value position
 */
int QBComboBoxGetPosition(SvWidget w);

/**
 * Get selected value.
 * If custom value is set, then it is returned.
 *
 * @param[in]   w   ComboBox widget handle
 * @return          selected value
 */
SvObject QBComboBoxGetValue(SvWidget w);

/**
 * Set custom value.
 *
 * Releases previous custom value if set.
 * If the value exists in the content, then the position is set to this value.
 *
 * Calls 'prepare' and 'current' callbacks.
 *
 * @param[in]   w       ComboBox widget handle
 * @param[in]   value   new custom value (not NULL)
 */
void QBComboBoxSetValue(SvWidget w, SvObject value);

/**
 * Replace content with new one.
 *
 * After setting new content, calls 'change' callback with old value.
 * Then calls 'prepare' and 'current' callbacks with the new value.
 *
 * @param[in]   w       ComboBox widget handle
 * @param[in]   content new content
 */
void QBComboBoxSetContent(SvWidget w, SvArray content);

void QBComboBoxSetDataSource(SvWidget w, SvObject dataSource);
SvObject QBComboBoxGetDataSource(SvWidget w);

/**
 * Set the position.
 *
 * If the position is negative, then it behaves like it's counting from the end
 * (-1 is the last value).
 * If the position is bigger than the number of values, then it's set to the
 * last one. Similarly if it's smaller than negative number of values, then
 * it's set to the first one.
 *
 * If custom value is set, then it's not changed (it does not affect changing
 * the position).
 *
 * Calls 'prepare' and 'current' callbacks.
 *
 * @param[in]   w   ComboBox widget handle
 * @param[in]   pos position
 */
void QBComboBoxSetPosition(SvWidget w, int pos);

/**
 * Set callbacks.
 *
 * @param[in]   w           ComboBox widget handle
 * @param[in]   target      pointer passed to callbacks as target argument
 * @param[in]   callbacks   structure with pointers to QBComboBox callbacks
 */
void QBComboBoxSetCallbacks(SvWidget w, void *target, QBComboBoxCallbacks callbacks);

/**
 * Reinitialize the combobox.
 *
 * Recreates the displayed text. Calls 'prepare' callback.
 *
 * @param[in]   w   ComboBox widget handle
 */
void QBComboBoxReinitialize(SvWidget w);

/**
 * Get contents of combobox.
 *
 * @param[in]   w   ComboBox widget handle
 * @return          content
 */
SvArray QBComboBoxGetContent(SvWidget w);

/**
 * Check if custom value set.
 *
 * @param[in]   w   ComboBox widget handle
 * @return          true if custom value is set
 */
bool QBComboBoxIsUserValue(SvWidget w);

/**
 * @}
 */

#endif

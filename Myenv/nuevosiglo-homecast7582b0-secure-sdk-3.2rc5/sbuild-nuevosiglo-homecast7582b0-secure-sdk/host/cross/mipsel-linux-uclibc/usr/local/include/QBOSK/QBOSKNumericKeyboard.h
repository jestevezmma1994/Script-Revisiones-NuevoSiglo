/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_OSK_NUMERIC_KEYBOARD_H_
#define QB_OSK_NUMERIC_KEYBOARD_H_

/**
 * @file QBOSKNumericKeyboard.h
 * @brief Numeric keyboard (SMS-like) typing logic
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

typedef struct QBOSKNumericKeyboard_ *QBOSKNumericKeyboard;

/**
 * @defgroup QBOSKNumericKeyboard Keyboard with SMS-like typing style
 * @ingroup QBOSK
 * @{
 *
 * Component for QBOSKNumericKeyboard
 *
 * It uses JSON file with RCU key mapping to letters/signs (UTF8) in NumericKeyboardMap.json file.
 * JSON description (with ;comments):
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.json}
 *    {
 *        "layout_name": [              ; array of buttons on RCU (i.e. 0-9)
 *        [                             ; items array of one button
 *            "2", ["a", "b", "c", "2"] ; first element of button array means button name (button identifier)
 *                                      ; second element is array of letters/signs which should be assigned to button
 *        ],
 *        [
 *            "3", ["d", "e", "f", "3"]
 *        ],
 *
 *        ...
 *    }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 **/

/**
 * Create new Numeric keyboard instance.
 *
 * @param[in] app           CUIT application handle
 * @param[out] errorOut     error info
 * @return                  created object, @c NULL in case of error
 **/
QBOSKNumericKeyboard
QBOSKNumericKeyboardCreate(SvApplication app,
                           SvErrorInfo *errorOut);

/**
 * Set proper layout name for numeric keyboard
 * If given layout name not exists for Numeric Keyboard - leave last layout.
 *
 * @param[in]     self      QBOSKNumericKeyboard handle
 * @param[in]     layoutID  name of layout to set, set @c to NULL if want to disable layout
 **/
void
QBOSKNumericKeyboardSetLayoutID(QBOSKNumericKeyboard self, SvString layoutID);

/**
 * Decode letter from pressed button
 *
 * @param[in]       self     QBOSKNumericKeyboard handle
 * @param[in]       button   pressed button code
 * @param[out]      errorOut error info
 * @return          letter for specified button; @c NULL if no defined layout
 **/
SvString
QBOSKNumericKeyboardGetLetterFromPressedButton(QBOSKNumericKeyboard self,
                                               int button,
                                               SvErrorInfo *errorOut);

/**
 * Check if given key is equal to the last
 *
 * @param[in]       self     QBOSKNumericKeyboard handle
 * @param[in]       button   pressed button code
 * @return          @c true when given button number is the same as last, @c false otherwise
 **/
bool
QBOSKNumericKeyboardKeyIsEqualToLast(QBOSKNumericKeyboard self, int button);

/**
 * Reset last pressed/decoded button and letter
 *
 * @param[in]       self      QBOSKNumericKeyboard handle
 **/
void
QBOSKNumericKeyboardResetKeys(QBOSKNumericKeyboard self);

/**
 * @}
 **/

#endif /* QB_OSK_NUMERIC_KEYBOARD_H_ */

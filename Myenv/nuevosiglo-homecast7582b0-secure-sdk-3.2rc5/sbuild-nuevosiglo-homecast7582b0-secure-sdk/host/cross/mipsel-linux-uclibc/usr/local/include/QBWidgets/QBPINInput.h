/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PIN_INPUT_H_
#define QB_PIN_INPUT_H_

/**
 * @file QBPINInput.h PIN input widget API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <CUIT/Core/types.h>


/**
 * @defgroup PINInput PIN input widget
 * @ingroup QBWidgets
 * @{
 **/

/**
 * @param[in] ptr      pointer to callback data
 * @param[in] sender   widget id which call notify
 * @param[in] isEnter  QBKEY_ENTER typed
 * @param[in] pin      typed pin
 **/
typedef void (*QBPINInputCallbackOnPIN)(void *ptr, SvWidgetId sender, bool isEnter, const char* pin);

struct QBPINInputCallbacks_t {
    QBPINInputCallbackOnPIN onPIN; ///< called when full PIN was typed or QBKEY_ENTER was typed
};

typedef struct QBPINInputCallbacks_t* QBPINInputCallbacks;

/**
 * Create PIN input widget using settings from the Settings Manager.
 *
 * This function creates PIN input widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 *  - width, height : integer, required,
 *  - horizSpacing : integer, optional (default: 0),
 *  - horizAlign : "center", "left" or "right" (default: "left"),
 *  - digitsCount : integer, optional (default: from constructor).
 *
 * PIN input widget creates a sub-widget for every digit, with name
 * created by appending the string ".Digit" to @a widgetName. Every
 * digit uses following parameters:
 *
 *  - width, height : integer, required,
 *  - bg, inactiveBg : bitmap, required,
 *  - marker : bitmap, required.
 *
 * @param[in] app          CUIT application context
 * @param[in] widgetName   widget name
 * @param[in] digitsCount  number of digits (pass @c 0 to get it from the
 *                         Settings Manager)
 * @param[in] callbacks    struct QBPINInputCallbacks with callbacks
 * @param[in] callbackData pointer to object, on it will be call callback
 * @param[out] errorOut    error info
 * @return                 created widget, @c NULL in case of error
 **/

extern SvWidget
QBPINInputNew(SvApplication app,
              const char *widgetName,
              unsigned int digitsCount,
              QBPINInputCallbacks callbacks,
              void *callbackData,
              SvErrorInfo *errorOut);

/**
 * Get current PIN value.
 *
 * @param[in] w             PIN input widget
 * @return                  current PIN value, @c NULL in case of error
 **/
extern const char *
QBPINInputGetValue(SvWidget w);

/**
 * Reset current PIN value.
 *
 * @param[in] w             PIN input widget
 **/
extern void
QBPINInputClear(SvWidget w);

/**
 * @}
 **/


#endif

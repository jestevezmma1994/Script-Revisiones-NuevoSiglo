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
#ifndef QBTIMEEDITBOX_H_
#define QBTIMEEDITBOX_H_

#include <stdbool.h>
#include <time.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <QBWidgets/QBComboBox.h>


typedef struct QBTimeEditBox_t* QBTimeEditBox;
typedef bool (*QBTimeEditBoxInputCallback)(void * target, SvWidget combobox, SvInputEvent e);
typedef SvString (*QBTimeEditBoxPrepareDataCallback)(void *target, SvWidget combobox, SvGenericObject value);
typedef int (*QBTimeEditBoxChangeCallback)(void *target, SvWidget combobox, SvGenericObject value, int key);

/**
 * Create new QBTimeEditBox from setttings.
 *
 * @param[in] app             application handle
 * @param[in] widgetName      widget name in settings file
 * @return                    newly created widget or @c NULL in case of error
 */
SvWidget QBTimeEditBoxNew(SvApplication app, const char *widgetName);

/**
 * Create new QBTimeEditBox from ratio setttings.
 *
 * @param[in] app             application handle
 * @param[in] widgetName      widget name in settings file
 * @param[in] parentWidth     parent width
 * @param[in] parentHeight    parent height
 * @return                    newly created widget or @c NULL in case of error
 */
SvWidget QBTimeEditBoxNewFromRatio(SvApplication app, const char *widgetName, unsigned int parentWidth, unsigned int parentHeight);

void QBTimeEditBoxSetFocus(SvWidget w);

int QBTimeEditBoxGetMinutes(SvWidget w);

void QBTimeEditBoxSetMinutes(SvWidget w, int m);

void QBTimeEditBoxSetTime(SvWidget w, time_t t);

bool QBTimeEditBoxIsFocused(SvWidget w);

void QBTimeEditBoxSetCallbacks(SvWidget w, void *target,
    QBComboBoxPrepareDataCallback prepare, QBComboBoxInputCallback input,
    QBComboBoxChangeCallback change);

#endif

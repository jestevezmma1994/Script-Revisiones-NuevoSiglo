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

#ifndef QB_CUIT_WIDGET_INTERNAL_H_
#define QB_CUIT_WIDGET_INTERNAL_H_

/**
 * @file QBCUITWidgetInternal.h QBCUITWidget internal API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/QBCUITWidget.h>

#include <CUIT/Core/QBWidgetInternal.h>
#include <CUIT/Core/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @addtogroup QBCUITWidget
 * @{
 **/

/**
 * QBCUITWidget class internals.
 **/
struct QBCUITWidget_ {
    struct QBWidget_ super_;    ///< super class

    SvWidget slave;            ///< slave widget
};

/**
 * Get runtime type identification object representing QBCUITWidget class.
 *
 * @relates QBCUITWidget
 *
 * @return QBCUITWidget runtime type identification object
 **/
extern SvType
QBCUITWidget_getType(void);

/**
 * Initialize widget with given width and height
 *
 * @memberof QBCUITWidget
 *
 * @param[in] self          widget handle
 * @param[in] app           application handle
 * @param[in] width         widget width
 * @param[in] height        widget height
 * @param[out] *errorOut    error info
 * @return                  self on success, @c NULL on error
 */
QBCUITWidget
QBCUITWidgetInit(QBCUITWidget self,
                 SvApplication app,
                 unsigned int width,
                 unsigned int height,
                 SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_CUIT_WIDGET_INTERNAL_H_ */

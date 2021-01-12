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

#ifndef QBHELPINFOWINDOW_H_
#define QBHELPINFOWINDOW_H_

/**
 * @file QBHelpInfoWindow.h Help Information Widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBWindowContext.h>
#include <Logic/InitLogic.h>
#include <Services/core/QBTextRenderer.h>

/**
 * Creates Help Informations window context.
 *
 * @param[in] initLogic         QBInitLogic handle
 * @param[in] textRenderer      text renderer handle
 * @return                      handle to created context, @c NULL if error occures.
 */
QBWindowContext
QBHelpInfoContextCreate(QBInitLogic initLogic,
                        QBTextRenderer textRenderer);

#endif //! QBHELPINFOWINDOW_H_

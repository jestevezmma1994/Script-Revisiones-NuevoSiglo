/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_ACCESS_MANAGER_LISTENER_STUB_H
#define QB_ACCESS_MANAGER_LISTENER_STUB_H

/**
 * @file QBAccessManagerListenerStub.h QBAccessManagerListenerStub class declaration file
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvType.h>

/**
 * @defgroup QBAccessManagerListenerStub stub for counting QBAccessManagerListener calls
 * @ingroup CubiTV_tests
 * @{
 */

/**
 * Access manager listener class for test purposes, which
 * just counts calls made to "authenticationChanged" method of QBAccessManagerListener interface.
 */
typedef struct QBAccessManagerListenerStub_* QBAccessManagerListenerStub;


/**
 * Get runtime type identification object
 * representing QBAccessManagerListenerStub class.
 * @return QBAccessManagerListenerStub type identification object
 **/
SvType QBAccessManagerListenerStub_getType(void);

/**
 * checks how many times listener was called
 *
 * @param self - this pointer
 * @return number of times listener was called
 */
int QBAccessManagerGetCallTimes(QBAccessManagerListenerStub self);

/**
 * @}
 **/

#endif //QB_ACCESS_MANAGER_LISTENER_STUB_H

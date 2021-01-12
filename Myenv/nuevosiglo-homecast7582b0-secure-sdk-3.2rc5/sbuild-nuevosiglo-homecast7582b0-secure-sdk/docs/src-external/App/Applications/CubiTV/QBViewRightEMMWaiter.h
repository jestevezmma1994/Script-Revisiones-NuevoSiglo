/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBVERIMATRIX_EMM_WAITER_H
#define QBVERIMATRIX_EMM_WAITER_H

#include <Logic/TVLogic.h>

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @file QBViewRightEMMWaiter.h
 * @brief AppStartupWaiter's plugin that waits for ViewRight library to catch as many EMMs as it needs to start descrambling
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBViewRightEMMWaiter ViewRight library initialization AppStartupWaiter plugin
 * @ingroup AppStartupWaiter_plugins
 * @{
 **/

/**
 * QBViewRightEMMWaiter class
 * Implements QBAppStartupWaiterPlugin interface.
 */
typedef struct QBViewRightEMMWaiter_s *QBViewRightEMMWaiter;

/**
 * Create AppStartupWaiter's plugin that waits for ViewRight library until it is ready to start descrambling
 *
 * @param[in] timeout timeout in seconds; 0 means infinity
 * @param[in] tvLogic pointer to QBTVLogic object
 * @param[out] errorOut          error info
 * @return  QBViewRightEMMWaiter instance, @c NULL in case of error
 */
QBViewRightEMMWaiter QBViewRightEMMWaiterCreate(unsigned timeout, QBTVLogic tvLogic, SvErrorInfo *errorOut);
/**
 * @}
 **/

#endif /* QBVERIMATRIX_EMM_WAITER_H */

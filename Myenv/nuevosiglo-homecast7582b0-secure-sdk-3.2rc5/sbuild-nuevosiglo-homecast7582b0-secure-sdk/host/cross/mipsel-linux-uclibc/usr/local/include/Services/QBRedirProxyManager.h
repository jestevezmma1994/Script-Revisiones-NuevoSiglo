/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBREDIRPROXYMANAGER_H
#define QBREDIRPROXYMANAGER_H

/**
 * @file QBRedirProxyManager.h
 * @brief Redirection proxy manager
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>
#include <Services/core/QBMiddlewareManager.h>

/**
 * @defgroup  QBRedirProxyManager Redirection proxy manager
 * @ingroup CubiTV_services
 * @{
 *
 * Redirection proxy manager handles situations when the middlewares are
 * behind proxy server. In that case all STBs connect to the common URL
 * and ask for the final middleware URL, which can be different for each
 * group of STBs. From now on they can start usual communication with the
 * middleware.
 * Operators use that feature to redirect groups of STBs to the nearest
 * middleware servers.
 *
 * This service tries to connect every @c requestsIntervalMs. If the
 * connection fails it enters a loop where it retries to connnect after
 * @c initialRetryIntervalMs, 2 * @c initialRetryIntervalMs, 4 *
 * @c initialRetryIntervalMs, 8 * @c initialRetryIntervalMs, etc, up to
 * @c maxRetryIntervalMs.
 *
 * Every time interval is multiplied by a random value between 0.5 and 1.5.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


/**
 * Redirection proxy manager class.
 **/
typedef struct QBRedirProxyManager_t *QBRedirProxyManager;


/**
 * Create redirection proxy manager
 *
 * @param[in] appGlobals                appGlobals handle
 * @param[in] initialRetryIntervalMs    initial interval between requests [ms]
 * @param[in] maxRetryIntervalMs        maximum interval between requests [ms]
 * @param[out] errorOut                 error info
 * @return                              created , @c NULL in case of error
 **/
extern QBRedirProxyManager
QBRedirProxyManagerCreate(AppGlobals appGlobals,
                          unsigned int initialRetryIntervalMs,
                          unsigned int maxRetryIntervalMs,
                          SvErrorInfo *errorOut);

/**
 * Start redirection proxy manager.
 *
 * @param[in] self        QBRedirProxyManager handle
 **/
extern void
QBRedirProxyManagerStart(QBRedirProxyManager self);

/**
 * Stop redirection proxy manager.
 *
 * @param[in] self        QBRedirProxyManager handle
 **/
extern void
QBRedirProxyManagerStop(QBRedirProxyManager self);

/**
 * Return final CubiwMW url as string.
 *
 * @param[in] self        QBRedirProxyManager handle
 * @return                final CubiMW url
 **/
extern SvString
QBRedirProxyManagerGetMiddlewareUrl(QBRedirProxyManager self);

/**
 * @}
 **/

#endif // QBREDIRPROXYMANAGER_H

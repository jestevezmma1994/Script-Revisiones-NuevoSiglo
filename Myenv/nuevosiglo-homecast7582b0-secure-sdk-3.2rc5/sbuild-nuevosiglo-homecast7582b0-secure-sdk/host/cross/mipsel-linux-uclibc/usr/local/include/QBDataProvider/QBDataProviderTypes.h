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

#ifndef QB_DATA_PROVIDER_TYPES_H_
#define QB_DATA_PROVIDER_TYPES_H_

/**
 * @file QBDataProviderTypes.h
 * @brief Data provider library types
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvTime.h>
#include <stdbool.h>

/**
 * Remote session authentication status.
 *
 * @struct QBRemoteSessionAuthStatus QBDataProviderTypes.h <QBDataProvider/QBDataProviderTypes.h>
 * @ingroup QBRemoteSessionManager
 **/
typedef struct QBRemoteSessionAuthStatus_ {
    /// @c true if session is authenticated
    bool isAuthenticated;

    /// @c true if authentication will never expire
    bool isAuthenticationPermanent;

    /// @c true if @a nextAuthenticationTime is valid
    bool isNextAuthenticationTimeKnown;

    /// time to start next authentication,
    /// before current authentication expires
    SvTime nextAuthenticationTime;
} QBRemoteSessionAuthStatus;


/**
 * Base class for session management.
 *
 * @class QBRemoteSessionManager QBRemoteSessionManager.h <QBDataProvider/QBRemoteSessionManager.h>
 * @extends SvObject
 * @implements QBObserver
 * @ingroup QBRemoteSessionManager
 *
 * @note If only class declaration needed, include <QBDataProvider/QBDataProviderTypes.h>
 *
 * Extend this class when custom handshake phase needs to be imlemented
 * (see QBRemoteSessionManagerVTable::performHandshake).
 **/
typedef struct QBRemoteSessionManager_ *QBRemoteSessionManager;

/**
 * Base class for QBRemoteSession authentication plugins.
 *
 * @class QBRemoteSessionAuthPlugin QBRemoteSessionAuthPlugin.h <QBDataProvider/QBRemoteSessionAuthPlugin.h>
 * @extends QBObservable
 * @ingroup QBRemoteSessionAuthPlugin
 **/
typedef struct QBRemoteSessionAuthPlugin_ *QBRemoteSessionAuthPlugin;

#endif /* QB_DATA_PROVIDER_TYPES_H_ */

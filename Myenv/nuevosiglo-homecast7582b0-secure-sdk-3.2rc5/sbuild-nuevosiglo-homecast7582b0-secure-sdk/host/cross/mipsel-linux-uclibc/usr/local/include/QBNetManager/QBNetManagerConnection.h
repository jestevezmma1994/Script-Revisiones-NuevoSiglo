/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* App/Libraries/QBNetManager/QBNetManager/QBNetManagerConnection.h */

#ifndef QB_NET_MANAGER_CONNECTION_H_
#define QB_NET_MANAGER_CONNECTION_H_

#include <stddef.h>
#include <stdint.h>
#include <SvFoundation/SvCoreTypes.h>


typedef struct QBNetManagerConnection_* QBNetManagerConnection;


struct QBNetManagerConnectionCallbacks_s {
    void (*teardown)(void *owner, QBNetManagerConnection connection);
};
typedef struct QBNetManagerConnectionCallbacks_s QBNetManagerConnectionCallbacks;


struct QBNetManagerConnection_ {
    struct SvObject_ super_;

    void *owner;
    QBNetManagerConnectionCallbacks* callbacks;

    //moredata
};

QBNetManagerConnection QBNetManagerConnectionCreate(void *owner, QBNetManagerConnectionCallbacks* callbacks);
void QBNetManagerConnectionTeardown(QBNetManagerConnection self);

// When the QBNetManagerConnection will depend on more data,
// it should be provided by QBNetManagerConnectionSet<more_data_name>
// Special attention should be paid for old code that setups the connections
// but doesn't call the new function. I other words, adding this function should be
// backward compatible.
// Also, connections attributes may be wildcard, so that some managers may register
// fewer connections, or they might not know the specifics of some connections.

#endif

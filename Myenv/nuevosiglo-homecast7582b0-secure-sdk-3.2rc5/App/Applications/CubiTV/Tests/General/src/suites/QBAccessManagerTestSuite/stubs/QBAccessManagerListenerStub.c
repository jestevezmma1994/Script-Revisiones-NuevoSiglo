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

#include "QBAccessManagerListenerStub.h"
#include <Services/QBAccessController/QBAccessManagerListener.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvLog.h>

struct QBAccessManagerListenerStub_ {
    struct SvObject_ super_;
    int callTimes;
};

SvLocal void QBAccessManagerListenerStub__dtor__(void *self_)
{
}

SvLocal void QBAccessManagerListenerStubAuthenticationChanged(SvObject self_,
                                                              QBAccessManager manager,
                                                              SvString domainName)
{
    QBAccessManagerListenerStub self = (QBAccessManagerListenerStub) self_;
    SvLogNotice("QBAccessManagerListenerStubAuthenticationChanged got authentication change signal for domain %s", SvStringCString(domainName));
    self->callTimes++;
}

SvType QBAccessManagerListenerStub_getType(void)
{
    static const struct QBAccessManagerListener_t methods = {
        .authenticationChanged = QBAccessManagerListenerStubAuthenticationChanged
    };
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBAccessManagerListenerStub__dtor__
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBAccessManagerListenerStub",
                            sizeof(struct QBAccessManagerListenerStub_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            QBAccessManagerListener_getInterface(), &methods,
                            NULL);
    }

    return type;
}

int QBAccessManagerGetCallTimes(QBAccessManagerListenerStub self)
{
    return self->callTimes;
}

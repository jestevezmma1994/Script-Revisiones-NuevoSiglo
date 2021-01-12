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

#include <Logic/SatelliteSetupLogic.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>

struct QBSatelliteSetupLogic_t {
    struct SvObject_ super_;
};

SvType QBSatelliteSetupLogic_getType(void)
{
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBSatelliteSetupLogic",
                            sizeof(struct QBSatelliteSetupLogic_t),
                            SvObject_getType(), &type, NULL);
    }
    return type;

}

QBSatelliteSetupLogic QBSatelliteSetupLogicNew(AppGlobals appGlobals)
{
    return (QBSatelliteSetupLogic) SvTypeAllocateInstance(QBSatelliteSetupLogic_getType(), NULL);
}

int QBSatelliteSetupLogicInputEventHandler(QBSatelliteSetupLogic self, const QBInputEvent* e)
{
    return 0;
}

bool QBSatelliteSetupLogicIsSetDiSEqCPortForAllSatellites(QBSatelliteSetupLogic self)
{
    return false;
}

bool QBSatelliteSetupLogicCanModifyDefaultSatellites(void)
{
    return true;
}

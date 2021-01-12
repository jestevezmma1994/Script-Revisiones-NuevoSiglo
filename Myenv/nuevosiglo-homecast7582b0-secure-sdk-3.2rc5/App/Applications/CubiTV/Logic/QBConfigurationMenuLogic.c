/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#include <Logic/QBConfigurationMenuLogic.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Services/QBRCUPairingService.h>
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvCoreTypes.h>
#include <Utils/appType.h>
#include <main_decl.h>

struct QBConfigurationMenuLogic_ {
    struct SvObject_ super_;
};

SvLocal SvType QBConfigurationMenuLogic_getType(void)
{
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBConfigurationMenuLogic",
                            sizeof(struct QBConfigurationMenuLogic_),
                            SvObject_getType(), &type, NULL);
    }
    return type;
}

QBConfigurationMenuLogic QBConfigurationMenuLogicCreate(AppGlobals appGlobals)
{
    return (QBConfigurationMenuLogic) SvTypeAllocateInstance(QBConfigurationMenuLogic_getType(), NULL);
}

bool QBConfigurationMenuLogicActionTrigger(QBConfigurationMenuLogic self, SvString action, QBActiveTreeNode node)
{
    if (!self || !action) {
        SvLogError("NULL passed as self (%p) or action (%p)", self, action);
        return false;
    }

    return false;
}

void QBConfigurationMenuLogicStart(QBConfigurationMenuLogic self)
{
    if (!self) {
        SvLogError("NULL passed as self");
        return;
    }
}

void QBConfigurationMenuLogicStop(QBConfigurationMenuLogic self)
{
    if (!self) {
        SvLogError("NULL passed as self");
        return;
    }
}

SvLocal void QBConfigurationMenuLogicRemoveNode(const SvString nodeName, QBActiveTree menu)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(menu, nodeName);
    if (node)
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
}

void QBConfigurationMenuLogicAdjustMenuTree(QBConfigurationMenuLogic self, QBActiveTree menu)
{
    if (QBAppTypeIsIP()) {
        QBConfigurationMenuLogicRemoveNode(SVSTRING("SearchByCategory"), menu);
        QBConfigurationMenuLogicRemoveNode(SVSTRING("DiagnosticsTunerStatistics"), menu);
        QBConfigurationMenuLogicRemoveNode(SVSTRING("DiagnosticsMPEGStatistics"), menu);
    }

    QBRCUPairingService rcuPairingService = (QBRCUPairingService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBRCUPairingService"));
    if (!QBRCUPairingServiceIsRF4CEAvailable(rcuPairingService)) {
        QBConfigurationMenuLogicRemoveNode(SVSTRING("RCUPairing"), menu);
    }
}

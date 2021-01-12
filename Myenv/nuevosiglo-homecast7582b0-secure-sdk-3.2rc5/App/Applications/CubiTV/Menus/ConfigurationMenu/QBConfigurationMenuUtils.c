/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBConfigurationMenuUtils.h"

#include <libintl.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <CUIT/Core/widget.h>
#include <QBConfig.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <settings.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBMenu/QBMenu.h>
#include <main.h>
#include <Utils/authenticators.h>
#include <Widgets/authDialog.h>
#include <Widgets/confirmationDialog.h>
#include <QBWidgets/QBDialog.h>

struct QBConfigurationMenuRestrictedNodesHandler_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    void *owner;
    SvWidget menu;
    SvWidget dialog;

    SvObject tree;
    SvObject node;
    SvObject path;

    QBConfigurationMenuItemNodeApply applyCallback;
};

SvLocal void
QBConfigurationMenuDialogCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBConfigurationMenuRestrictedNodesHandler self = self_;
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button") && self->node) {
        if (!self->applyCallback(self->owner, self->node, self->path)) {
            QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->tree, getIterator, self->path, 0);
            if (QBTreeIteratorGetNodesCount(&iter) > 0) {
                SvObject destPath = QBTreeIteratorGetCurrentNodePath(&iter);
                SvInvokeInterface(QBMenu, self->menu->prv, setPosition, destPath, NULL);
            }
        }
    }

    self->dialog = NULL;
    SVTESTRELEASE(self->node);
    SVTESTRELEASE(self->path);
    self->node = NULL;
    self->path = NULL;
}

SvLocal void
QBConfigurationMenuRestrictedNodesHandlerShowPCDialog(QBConfigurationMenuRestrictedNodesHandler self, SvObject node, SvObject path)
{
    SvString parentalControlText = NULL;
    if (SvObjectIsInstanceOf(node, QBActiveTreeNode_getType()))
        parentalControlText = (SvString) SVTESTRETAIN(QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("ParentalControlText")));
    if (!parentalControlText) {
        parentalControlText = SvStringCreate(gettext("Please enter parental control PIN"), NULL);
    }

    SVTESTRELEASE(self->node);
    SVTESTRELEASE(self->path);
    self->node = SVRETAIN(node);
    self->path = SVRETAIN(path);

    svSettingsPushComponent("ParentalControl.settings");
    SvObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC_MENU"));
    self->dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), gettext(SvStringCString(parentalControlText)), true, NULL, NULL);
    svSettingsPopComponent();

    QBDialogRun(self->dialog, self, QBConfigurationMenuDialogCallback);

    SVRELEASE(parentalControlText);
}

SvLocal void
QBConfigurationMenuRestrictedNodesHandlerShowConfirmationDialog(QBConfigurationMenuRestrictedNodesHandler self, SvObject node, SvObject path)
{
    SvString confirmationText = NULL;
    if (SvObjectIsInstanceOf(node, QBActiveTreeNode_getType()))
        confirmationText = (SvString) SVTESTRETAIN(QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("ConfirmationText")));
    if (!confirmationText) {
        confirmationText = SvStringCreate(gettext("Are you sure?"), NULL);
    }

    SVTESTRELEASE(self->node);
    SVTESTRELEASE(self->path);
    self->node = SVRETAIN(node);
    self->path = SVRETAIN(path);

    QBConfirmationDialogParams_t params = {
        .title                 = gettext("Confirmation Required"),
        .message               = gettext(SvStringCString(confirmationText)),
        .local                 = true,
        .focusOK               = false,
        .isCancelButtonVisible = true
    };
    self->dialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);
    QBDialogRun(self->dialog, self, QBConfigurationMenuDialogCallback);

    SVRELEASE(confirmationText);
}

SvLocal void
QBConfigurationMenuRestrictedNodesHandlerDestroy(void *self_)
{
    QBConfigurationMenuRestrictedNodesHandler self = self_;

    SVTESTRELEASE(self->tree);
    SVTESTRELEASE(self->node);
    SVTESTRELEASE(self->path);
}

SvLocal SvType
QBConfigurationMenuRestrictedNodesHandler_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConfigurationMenuRestrictedNodesHandlerDestroy
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConfigurationMenuRestrictedNodesHandler",
                            sizeof(struct QBConfigurationMenuRestrictedNodesHandler_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBConfigurationMenuRestrictedNodesHandler
QBConfigurationMenuRestrictedNodesHandlerCreate(AppGlobals appGlobals,
                                                void *owner,
                                                QBConfigurationMenuItemNodeApply applyCallback,
                                                SvWidget menu,
                                                QBActiveTree tree,
                                                SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!appGlobals || !owner || !applyCallback || !menu || !tree) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
        goto out;
    }

    QBConfigurationMenuRestrictedNodesHandler self = (QBConfigurationMenuRestrictedNodesHandler)
        SvTypeAllocateInstance(QBConfigurationMenuRestrictedNodesHandler_getType(), NULL);

    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBConfigurationMenuRestrictedNodesHandler");
        SVRELEASE(self);
        goto out;
    }
    self->owner = owner;
    self->menu = menu;
    self->tree = SVRETAIN(tree);
    self->appGlobals = appGlobals;
    self->applyCallback = applyCallback;

    return self;
out:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

QBConfigurationMenuTickState
QBConfigurationMenuCheckTickState(QBActiveTreeNode node)
{
    QBConfigurationMenuTickState tickState = QBConfigurationMenuTickState__Hidden;

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    SvString optName = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationName"));
    if (!optName && parent)
        optName = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName"));

    if (!optName)
        return tickState;

    SvString optVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));
    const char *value = QBConfigGet(SvStringCString(optName));
    SvLogNotice("CheckTickState [%s] + [%s] = [%s]",
                optName ? SvStringCString(optName) : "",
                optVal ? SvStringCString(optVal) : "",
                value);
    if (optVal) {
        if (value && strcasecmp(SvStringCString(optVal), value) == 0)
            tickState = QBConfigurationMenuTickState__On;
        else
            tickState = QBConfigurationMenuTickState__Off;
    }

    return tickState;
}

bool
QBConfigurationMenuHandleItemNodeSelection(QBConfigurationMenuRestrictedNodesHandler self,
                                           SvObject node_,
                                           SvObject path)
{
    if (!SvObjectIsInstanceOf((SvObject) node_, QBActiveTreeNode_getType()))
        return false;

    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString parentalControl = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("ParentalControl"));
    SvString confirmation = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("Confirmation"));

    if (QBConfigurationMenuCheckTickState(node) == QBConfigurationMenuTickState__On) {
        return true;
    }

    if (parentalControl && SvStringEqualToCString(parentalControl, "yes")) {
        QBConfigurationMenuRestrictedNodesHandlerShowPCDialog(self, node_, path);
        return true;
    }
    if (confirmation && SvStringEqualToCString(confirmation, "yes")) {
        QBConfigurationMenuRestrictedNodesHandlerShowConfirmationDialog(self, node_, path);
        return true;
    }

    return false;
}

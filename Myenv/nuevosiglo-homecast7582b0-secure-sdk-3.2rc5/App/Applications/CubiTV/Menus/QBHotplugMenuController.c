/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBHotplugMenuController.h"

#include <libintl.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Services/QBFileBrowserMounter.h>
#include <Services/QBPVRMounter.h>
#include "Widgets/QBMenuCaptionGetter.h"
#include <XMB2/XMBItemController.h>
#include <main.h>
#include <Utils/productUtils.h>

struct QBHotplugMenuController_t {
    struct SvObject_ super_;

    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBFrameConstructor* disabledFocus;
    QBPVRMounter mounter;
    QBFileBrowserMounter fileBrowserMounter;
    QBTextRenderer renderer;

    SvBitmap iconDisk;
    SvBitmap iconPart;
    SvRID iconRID;

    AppGlobals appGlobals;
};

SvLocal SvString QBHotplugMenuControllerGetCaption(SvGenericObject self_, SvGenericObject path, SvGenericObject node_)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    SvString caption = NULL;
    QBPartitionInfo partInfo = (QBPartitionInfo) QBActiveTreeNodeGetAttribute(node, SVSTRING("QBPartitionInfo"));
    QBDiskInfo diskInfo = (QBDiskInfo) QBActiveTreeNodeGetAttribute(node, SVSTRING("QBDiskInfo"));
    if(partInfo) {
        caption = partInfo->label;
    } else if(diskInfo) {
        double size_gb = (double) diskInfo->size / (1024 * 1024 * 1024);
        caption = SvStringCreateWithFormat("%s : %2.2f GB", SvStringCString(diskInfo->model), size_gb);
        SVAUTORELEASE(caption);
    }
    return caption;
}


SvLocal SvWidget QBHotplugMenuControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    QBHotplugMenuController self = (QBHotplugMenuController) self_;

    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString id = QBActiveTreeNodeGetID(node);
    QBPartitionInfo partInfo = (QBPartitionInfo) QBActiveTreeNodeGetAttribute(node, SVSTRING("QBPartitionInfo"));


    QBXMBItem item = QBXMBItemCreate();

    SvString caption = QBHotplugMenuControllerGetCaption((SvGenericObject) self, path, node_);
    item->caption = SVTESTRETAIN(caption);

    if (partInfo) {
        double size_gb = (double) partInfo->size / (1024 * 1024 * 1024);
        if (partInfo->type)
            item->subcaption = SvStringCreateWithFormat("%s - %2.2f GB", SvStringCString(partInfo->type), size_gb);
        else
            item->subcaption = SvStringCreateWithFormat("%2.2f GB", size_gb);
        if (!item->icon) {
            item->icon = SVTESTRETAIN(self->iconPart);
        }
    } else {
        if(self->mounter) {
            QBDiskStatus diskStat = QBPVRMounterGetDiskStatus(self->mounter, id);
            QBPVRMounterState state = QBPVRMounterGetState(self->mounter, id);
            if(state == QBPVRMounterState_disk_mounted) {
                if (diskStat->pvr && diskStat->ts)
                    item->subcaption = SvStringCreate(gettext("Used for PVR and Timeshift"), NULL);
                else if (diskStat->ts)
                    item->subcaption = SvStringCreate(gettext("Used for Timeshift"), NULL);
                else
                    item->subcaption = SvStringCreate(gettext("Used for PVR"), NULL);
            } else if(state == QBPVRMounterState_disk_compatible) {
                if (diskStat->pvr && diskStat->ts)
                    item->subcaption = SvStringCreate(gettext("Compatible with PVR and Timeshift"), NULL);
                else if (diskStat->ts)
                    item->subcaption = SvStringCreate(gettext("Compatible with Timeshift"), NULL);
                else
                    item->subcaption = SvStringCreate(gettext("Compatible with PVR"), NULL);
            } else if(state == QBPVRMounterState_disk_formatting)
                item->subcaption = SvStringCreate(gettext("Formatting..."), NULL);
            else if(state == QBPVRMounterState_disk_mounting)
                item->subcaption = SvStringCreate(gettext("Mounting..."), NULL);
            else if(state == QBPVRMounterState_disk_unmounting)
                item->subcaption = SvStringCreate(gettext("Unmounting..."), NULL);
            else if(state == QBPVRMounterState_error_mounting)
                item->subcaption = SvStringCreate(gettext("Non compatible storage device detected (error 1)"), NULL);
            else if(state == QBPVRMounterState_disk_incompatible) {
                if (self->fileBrowserMounter) {
                    if (QBPVRLogicIsDiskPVRCompatible(diskStat->disk))  // for PVR compatible disks only
                        item->subcaption = SvStringCreate(gettext("Press OK to browse or OPT to format"), NULL);
                    else
                        item->subcaption = SvStringCreate(gettext("Press OK to browse"), NULL);
                }
                else
                    item->subcaption = SvStringCreate(gettext("Press OPT to format"), NULL);
            } else if(state == QBPVRMounterState_error_unmounting)
                item->subcaption = SvStringCreate(gettext("Could not stop device, please try again (error 2)"), NULL);
            else if(state == QBPVRMounterState_error_formatting)
                item->subcaption = SvStringCreate(gettext("Non compatible storage device detected, (error 3)"), NULL);
            else if(state == QBPVRMounterState_disk_checking)
                item->subcaption = SvStringCreate(gettext("Checking compatibility..."), NULL);
        } else if (self->fileBrowserMounter) {
            item->subcaption = SvStringCreate(gettext("Press OK to browse"), NULL);
        }
        if (!item->icon)
            item->icon = SVTESTRETAIN(self->iconDisk);
    }

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    item->disabledFocus = SVTESTRETAIN(self->disabledFocus);

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void QBHotplugMenuControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBHotplugMenuController self = (QBHotplugMenuController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBHotplugMenuControllerDestroy(void *self_)
{
    QBHotplugMenuController self = self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVTESTRELEASE(self->iconDisk);
    SVTESTRELEASE(self->iconPart);
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->disabledFocus);
}

SvLocal SvType QBHotplugMenuController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBHotplugMenuControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t controller_methods = {
        .createItem = QBHotplugMenuControllerCreateItem,
        .setItemState = QBHotplugMenuControllerSetItemState,
    };

    static const struct QBMenuCaptionGetter_t caption_methods = {
        .getMenuCaptionForPath = QBHotplugMenuControllerGetCaption,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHotplugMenuController",
                            sizeof(struct QBHotplugMenuController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            QBMenuCaptionGetter_getInterface(), &caption_methods,
                            NULL);
    }

    return type;
}


QBHotplugMenuController QBHotplugMenuControllerCreate(AppGlobals appGlobals)
{
    QBHotplugMenuController self = (QBHotplugMenuController) SvTypeAllocateInstance(QBHotplugMenuController_getType(), NULL);

    self->appGlobals = appGlobals;
    self->itemConstructor = QBXMBItemConstructorCreate("HotplugItem", appGlobals->textRenderer);

    self->iconDisk = SVTESTRETAIN(svSettingsGetBitmap("HotplugItem", "iconDisk"));
    self->iconPart = SVTESTRETAIN(svSettingsGetBitmap("HotplugItem", "iconPart"));
    self->iconRID = svSettingsGetResourceID("HotplugItem", "iconDisk");
    self->focus = QBFrameConstructorFromSM("HotplugItem.focus");
    if(svSettingsIsWidgetDefined("HotplugItem.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("HotplugItem.inactiveFocus");
    if(svSettingsIsWidgetDefined("HotplugItem.disabledFocus"))
        self->disabledFocus = QBFrameConstructorFromSM("HotplugItem.disabledFocus");
    self->mounter = appGlobals->PVRMounter;
    self->fileBrowserMounter = appGlobals->fileBrowserMounter;
    self->renderer = appGlobals->textRenderer;

    return self;
}


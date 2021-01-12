/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBSatelliteSetupEditorPane.h"

#include <QBSecureLogManager.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <ContextMenus/QBAutoMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBListPane.h>
#include <ContextMenus/QBSatelliteEditPane.h>
#include <ContextMenus/QBTransponderEditPane.h>
#include <Menus/QBChannelChoiceMenu.h>
#include <QBConf.h>
#include <QBDVBSatelliteDescriptor.h>
#include <QBDVBSatellitesDB.h>
#include <QBDataModel3/QBActiveArray.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBListModelListener.h>
#include <QBInput/QBInputCodes.h>
#include <QBTunerLNBConfig.h>
#include <QBTunerTypes.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvComparator.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <Utils/translations.h>
#include <Widgets/confirmationDialog.h>
#include <QBWidgets/QBDialog.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <assert.h>
#include <libintl.h>
#include <main.h>
#include <main_decl.h>
#include <settings.h>
#include <string.h>
#include <sys/types.h>

typedef enum {
    QBSatelliteSetupEditorPaneConfirmation_NONE,
    QBSatelliteSetupEditorPaneConfirmation_EDIT_BASIC_DATA,
    QBSatelliteSetupEditorPaneConfirmation_TOOGLE_TRANSPONDERS,
    QBSatelliteSetupEditorPaneConfirmation_REMOVE_TRANSPONDERS
} QBSatelliteSetupEditorPaneConfirmation;

typedef enum {
    QBSatelliteSetupNotification_ITEM_ADDED,
    QBSatelliteSetupNotification_ITEM_REMOVED,
    QBSatelliteSetupNotification_ITEM_CHANGED,
    QBSatelliteSetupNotification_ITEMS_REORDERED,
} QBSatelliteSetupNotification;

struct QBSatelliteSetupEditorPane_ {
    struct SvObject_ super_;

    SvApplication app;
    QBDVBSatellitesDB satellitesDB;
    QBContextMenu contextMenu;

    QBAutoMenu options;

    SvArray satellitesList;
    QBDVBSatelliteDescriptor selectedSatellite;
    QBBasicSatelliteData newSatellite;

    int settingsCtx;

    QBActiveArray allTransponders;
    SvHashTable tickedTransponders;
    QBListPane transpondersSelectionPane;

    QBDVBSatelliteTransponder selectedTransponder;
    int selectedTransponderPosition;

    QBSatelliteSetupEditorPaneConfirmation confirmationType;

    SvWidget dialog;

    SvWeakReference listener;
};


SvLocal int
compareTransponders(void *prv,
                    SvObject obj1,
                    SvObject obj2)
{
    // QBSatelliteSetupEditor::allTransponders array contains also SvString for 'Add transponder' menu item
    if (!SvObjectIsInstanceOf(obj1, QBDVBSatelliteTransponder_getType()))
        return -1;
    if (!SvObjectIsInstanceOf(obj2, QBDVBSatelliteTransponder_getType()))
        return 1;

    return QBDVBSatelliteTransponderCompare(prv, obj1, obj2);
}

SvLocal ssize_t
QBActiveArrayIndexOfObjectFast(QBActiveArray self,
                               size_t minIdx, size_t maxIdx,
                               SvObject object,
                               SvObjectCompareFn compare,
                               void *prv)
{
    minIdx = QBActiveArrayLowerBound(self, minIdx, maxIdx, object, compare, prv);
    SvObject tmp = QBActiveArrayObjectAtIndex(self, minIdx);
    if (compare(prv, tmp, object) == 0)
        return minIdx;
    else
        return -1;
}

SvLocal SvString
createTransponderCaption(QBDVBSatelliteTransponder transponder)
{
    return SvStringCreateWithFormat(gettext("%dMHz,%s,%d,%s"),
                                    transponder->desc.mux_id.freq / 1000,
                                    QBTunerPolarizationToTranslatedShortString(transponder->desc.polarization),
                                    transponder->desc.symbol_rate,
                                    QBTunerModulationToTranslatedString(transponder->desc.modulation));
}

SvLocal SvString
QBBasicSatelliteDataGetFullName(QBBasicSatelliteData self)
{
    return SvStringCreateWithFormat(gettext("%s (%d.%d%s)"), SvStringCString(self->name),
                                    self->deg / 10, self->deg % 10,
                                    SvStringCString(self->hemisphere));
}

SvLocal unsigned int
QBBasicSatelliteDataGetPosition(QBBasicSatelliteData self)
{
    unsigned int position;
    if (SvObjectEquals((SvObject) self->hemisphere, (SvObject) SVSTRING("W"))) {
        position = 3600 - self->deg;
    } else {
        position = self->deg;
    }

    return position;
}

SvLocal void
QBSatelliteSetupEditorPaneNotify(QBSatelliteSetupEditorPane self, int idx, int cnt, QBSatelliteSetupNotification notification)
{
    if (self->listener) {
        SvObject list = SvWeakReferenceTakeReferredObject(self->listener);
        if (!list) {
            SVRELEASE(self->listener);
            self->listener = NULL;
            return;
        }

        switch (notification) {
            case QBSatelliteSetupNotification_ITEM_ADDED:
                SvInvokeInterface(QBListModelListener, list, itemsAdded, idx, cnt);
                break;
            case QBSatelliteSetupNotification_ITEM_REMOVED:
                SvInvokeInterface(QBListModelListener, list, itemsRemoved, idx, cnt);
                break;
            case QBSatelliteSetupNotification_ITEM_CHANGED:
                SvInvokeInterface(QBListModelListener, list, itemsChanged, idx, cnt);
                break;
            case QBSatelliteSetupNotification_ITEMS_REORDERED:
                SvInvokeInterface(QBListModelListener, list, itemsReordered, idx, cnt);
                break;
        }
        SVRELEASE(list);
    }
}

SvLocal void
QBSatelliteSetupEditorPaneNotifyWithChange(QBSatelliteSetupEditorPane self)
{
    ssize_t satIdx = SvArrayIndexOfObject(self->satellitesList, (SvObject) self->selectedSatellite);
    QBSatelliteSetupEditorPaneNotify(self, satIdx, 1, QBSatelliteSetupNotification_ITEM_CHANGED);
}

SvLocal SvWidget
QBSatelliteSetupEditorPaneCreatePopup(QBSatelliteSetupEditorPane self,
                                      const char *title,
                                      const char *message)
{
    QBConfirmationDialogParams_t params = {
        .title                 = title,
        .message               = message,
        .local                 = true,
        .globalName            = NULL,
        .focusOK               = true,
        .isCancelButtonVisible = false
    };

    SvWidget confirmationDialog = QBConfirmationDialogCreate(self->app, &params);

    return confirmationDialog;
}

SvLocal void
QBSatelliteSetupEditorPaneTransponderPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBSatelliteSetupEditorPane self = (QBSatelliteSetupEditorPane) self_;
    self->dialog = NULL;
}

SvLocal void
QBSatelliteSetupEditorPaneTransponderShowPopup(QBSatelliteSetupEditorPane self, int errorType)
{
    const char* type;
    const char* title;
    const char* message;
    switch (errorType) {
        case QBDVBSatelliteTransponderCheck_invalidFrequency:
            type = "ErrorShown.ContextMenus.InvalidTransponderFrequency";
            title = gettext("Invalid tranponder");
            message = gettext("Transponder has incorrect frequency value.");
            break;
        case QBDVBSatelliteTransponderCheck_invalidSymbolRate:
            type = "ErrorShown.ContextMenus.InvalidTransponderSymbolRate";
            title = gettext("Invalid tranponder");
            message = gettext("Transponder has incorrect symbol rate value.");
            break;
        case QBDVBSatelliteTransponderCheck_duplicate:
            type = "ErrorShown.ContextMenus.TransponderDuplicate";
            title = gettext("Duplicate transponder");
            message = gettext("Transponder is duplicate (has the same frequency as existing).");
            break;
        default:
            type = "ErrorShown.ContextMenus.UnknownTransponderError";
            title = gettext("Error");
            message = gettext("Unknown error when processing transponder.");
    }
    self->dialog = QBSatelliteSetupEditorPaneCreatePopup(self, title, message);
    QBSecureLogEvent("SatelliteSetup", type, "JSON:{\"description\":\"%s\"}", message);
    QBDialogRun(self->dialog, self, QBSatelliteSetupEditorPaneTransponderPopupCallback);
}

SvLocal void
QBSatelliteSetupEditorPaneSatelliteSortSatellitesList(QBSatelliteSetupEditorPane self, SvString optVal)
{
    bool newSort = false;
    if (optVal == NULL) {
        SvLogError("%s(): NULL argument passed", __func__);
        return;
    }
    if (SvStringEqualToCString(optVal, "letter")) {
        QBDVBSatelliteDescriptorSortByName(self->satellitesList);
        newSort = true;
    } else if (SvStringEqualToCString(optVal, "position")) {
        QBDVBSatelliteDescriptorSortByPosition(self->satellitesList);
        newSort = true;
    }
    if (newSort) {
        QBConfigSet("SATSORTTYPE", SvStringCString(optVal));
        QBConfigSave();
        QBSatelliteSetupEditorPaneNotify(self, 0, SvArrayCount(self->satellitesList),
                                         QBSatelliteSetupNotification_ITEMS_REORDERED);
    } else {
        SvLogError("%s(): wrong argument passed: %s", __func__, SvStringGetCString(optVal));
    }
}

SvLocal ChannelChoiceTickState
QBSatelliteSetupEditorPaneSatelliteSortTypeIsTicked(void *self_, SvObject obj)
{
    SvString sortType = (SvString) obj;

    const char *currentSortType = QBConfigGet("SATSORTTYPE");

    if (currentSortType && SvStringEqualToCString(sortType, currentSortType))
        return ChannelChoiceTickState_On;
    return ChannelChoiceTickState_Off;
}

SvLocal SvString
QBSatelliteSetupEditorPaneSatelliteSortTypeCreateCaption(void *self_, SvObject obj)
{
    SvString sortType = (SvString) obj;

    if (SvStringEqualToCString(sortType, "letter"))
        return SvStringCreate(gettext("Alphabetically"), NULL);
    if (SvStringEqualToCString(sortType, "position"))
        return SvStringCreate(gettext("By Longitude"), NULL);

    SvLogError("%s() : Unknown sort type", __func__);
    return NULL;
}

SvLocal void
QBSatelliteSetupEditorPaneSatelliteSortTypeSelected(void *self_,
                                                    QBListPane pane,
                                                    SvObject obj,
                                                    int pos)
{
    QBSatelliteSetupEditorPane self = self_;
    SvString sortType = (SvString) obj;

    if (!SvObjectIsInstanceOf(obj, SvString_getType()))
        return;

    QBSatelliteSetupEditorPaneSatelliteSortSatellitesList(self, sortType);
    QBContextMenuHide(self->contextMenu, false);
}


SvLocal void
QBSatelliteSetupEditorPaneSatelliteSortShowPane(QBSatelliteSetupEditorPane self, int level)
{
    static struct QBChannelChoiceControllerCallbacks_t callbacks = {
        .isTicked          = QBSatelliteSetupEditorPaneSatelliteSortTypeIsTicked,
        .createNodeCaption = QBSatelliteSetupEditorPaneSatelliteSortTypeCreateCaption
    };

    static struct QBListPaneCallbacks_t listCallbacks = {
        .selected = QBSatelliteSetupEditorPaneSatelliteSortTypeSelected
    };

    QBActiveArray satSortTypes = QBActiveArrayCreate(2, NULL);

    QBActiveArrayAddObject(satSortTypes, (SvObject) SVSTRING("letter"), NULL);
    QBActiveArrayAddObject(satSortTypes, (SvObject) SVSTRING("position"), NULL);

    QBListPane satSortTypesPane = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));
    QBChannelChoiceController constructor =
        QBChannelChoiceControllerCreateFromSettings("ChannelChoiceMenu.settings",
                                                    textRenderer,
                                                    (SvObject) satSortTypes,
                                                    self, &callbacks);

    svSettingsRestoreContext(self->settingsCtx);
    QBListPaneInit(satSortTypesPane, self->app, (SvObject) satSortTypes,
                   (SvObject) constructor, &listCallbacks, self, self->contextMenu,
                   level, SVSTRING("ListPane"));
    svSettingsPopComponent();

    SVRELEASE(constructor);
    SVRELEASE(satSortTypes);

    QBContextMenuPushPane(self->contextMenu, (SvObject) satSortTypesPane);
    SVRELEASE(satSortTypesPane);
}

SvLocal size_t
QBSatelliteSetupEditorPaneSatelliteAdd(QBSatelliteSetupEditorPane self, QBDVBSatelliteDescriptor desc)
{
    QBDVBSatelliteDescriptor tmpDesc;
    int (*localCompareFn)(QBDVBSatelliteDescriptor descA, QBDVBSatelliteDescriptor descB);

    const char *sortType = QBConfigGet("SATSORTTYPE");
    if (!sortType) {
        SvArrayAddObject(self->satellitesList, (SvObject) desc);
        return SvArrayCount(self->satellitesList) - 1;
    }

    if (strcmp(sortType, "position") == 0)
        localCompareFn = QBDVBSatelliteDescriptorCompareByPosition;
    else
        localCompareFn = QBDVBSatelliteDescriptorCompareByName;

    size_t idx = 0;
    SvIterator it = SvArrayIterator(self->satellitesList);
    while ((tmpDesc = (QBDVBSatelliteDescriptor) SvIteratorGetNext(&it))) {
        if (localCompareFn(desc, tmpDesc) <= 0)
            break;
        idx++;
    }

    SvArrayInsertObjectAtIndex(self->satellitesList, (SvObject) desc, idx);
    return idx;
}

SvLocal void
QBSatelliteSetupEditorPaneSatelliteEditSaveNew(QBSatelliteSetupEditorPane self)
{
    unsigned int position = QBBasicSatelliteDataGetPosition(self->newSatellite);
    SvString name = QBBasicSatelliteDataGetFullName(self->newSatellite);

    QBDVBSatelliteDescriptor satellite = QBDVBSatellitesDBCreateEntry(self->satellitesDB,
                                                                      name,
                                                                      position,
                                                                      NULL);
    SVRELEASE(name);
    if (!satellite) {
        SvLogError("%s(): Can't create new satellite", __func__);
        return;
    }

    int newIdx = QBSatelliteSetupEditorPaneSatelliteAdd(self, satellite);
    QBSatelliteSetupEditorPaneNotify(self, newIdx, 1, QBSatelliteSetupNotification_ITEM_ADDED);
    self->selectedSatellite = satellite;
    SVRELEASE(satellite);
    self->newSatellite = NULL;
}

SvLocal void
QBSatelliteSetupEditorPaneSatelliteEditSaveBasicData(QBSatelliteSetupEditorPane self, QBBasicSatelliteData satelliteData, QBDVBSatelliteDescriptor satellite)
{
    SvString newName = NULL;
    if (!satelliteData || !satelliteData->name || !satelliteData->hemisphere || satelliteData->deg == -1)
        goto err;

    int newIdx;

    bool newSatellite = satellite == NULL;
    if (newSatellite) {
        self->newSatellite = satelliteData;
    } else {
        unsigned int position = QBBasicSatelliteDataGetPosition(satelliteData);
        newName = QBBasicSatelliteDataGetFullName(satelliteData);

        SvString oldName = QBDVBSatelliteDescriptorGetName(satellite);
        bool nameChanged = !SvObjectEquals((SvObject) oldName, (SvObject) newName);
        bool posChanged = QBDVBSatelliteDescriptorGetPosition(satellite) != position;

        if (!nameChanged && !posChanged)
            goto err;

        if (nameChanged)
            QBDVBSatelliteDescriptorSetName(satellite, newName);

        if (posChanged)
            QBDVBSatelliteDescriptorSetPosition(satellite, position);

        SvArrayRemoveObject(self->satellitesList, (SvObject) satellite);
        newIdx = QBSatelliteSetupEditorPaneSatelliteAdd(self, satellite);

        QBDVBSatellitesDBSaveEntry(self->satellitesDB, satellite);
        QBSatelliteSetupEditorPaneNotify(self, newIdx, 1, QBSatelliteSetupNotification_ITEM_CHANGED);
    }

err:
    SVTESTRELEASE(newName);
}

SvLocal void
QBSatelliteSetupEditorPaneTransponderAdded(void *self_,
                                           QBTransponderEditPane pane,
                                           QBBasicTransponderData transponderData);

SvLocal
void QBSatelliteSetupEditorPaneTransponderShowEditor(QBSatelliteSetupEditorPane self,
                                                     unsigned int level,
                                                     QBTransponderEditCallback callback,
                                                     QBDVBSatelliteTransponder transponder)
{
    svSettingsRestoreContext(self->settingsCtx);
    QBTransponderEditPane tpEdit =
        QBTransponderEditPaneCreate(self->contextMenu, self->app,
                                    level + 1, transponder,
                                    callback,
                                    self, NULL);
    svSettingsPopComponent();

    QBContextMenuPushPane(self->contextMenu, (SvObject) tpEdit);
    SVRELEASE(tpEdit);
}

SvLocal
void QBSatelliteSetupEditorPaneSatelliteAdded(void *self_, QBSatelliteEditPane pane, QBBasicSatelliteData satelliteData)
{
    QBSatelliteSetupEditorPane self = self_;

    QBSatelliteSetupEditorPaneSatelliteEditSaveBasicData(self, satelliteData, NULL);
    unsigned int level = QBSatelliteEditPaneGetLevel(pane);
    QBSatelliteSetupEditorPaneTransponderShowEditor(self, level, QBSatelliteSetupEditorPaneTransponderAdded, NULL);
}

SvLocal
void QBSatelliteSetupEditorPaneSatelliteEdited(void *self_, QBSatelliteEditPane pane, QBBasicSatelliteData satelliteData)
{
    QBSatelliteSetupEditorPane self = self_;

    QBSatelliteSetupEditorPaneSatelliteEditSaveBasicData(self, satelliteData, self->selectedSatellite);
    QBContextMenuPopPane(self->contextMenu);
}

SvLocal SvString
QBSatelliteSetupEditorPaneCreateTransponderCaption(void *self_, SvObject obj)
{
    if (SvObjectIsInstanceOf(obj, QBDVBSatelliteTransponder_getType())) {
        return createTransponderCaption((QBDVBSatelliteTransponder) obj);
    }
    if (SvObjectIsInstanceOf(obj, SvString_getType())) {
        return (SvString) SVRETAIN(obj);
    }
    return NULL;
}

SvLocal ChannelChoiceTickState
QBSatelliteSetupEditorPaneTransponderIsTicked(void *self_, SvObject transponder)
{
    QBSatelliteSetupEditorPane self = self_;
    if (SvHashTableFind(self->tickedTransponders, transponder)) {
        return ChannelChoiceTickState_On;
    }

    return ChannelChoiceTickState_Off;
}

SvLocal ChannelChoiceTickState
QBSatelliteSetupEditorPaneTransponderSingleEditIsTicked(void *self_, SvObject transponder_)
{
    QBDVBSatelliteTransponder transponder = (QBDVBSatelliteTransponder) transponder_;
    return transponder->isEnabled ? ChannelChoiceTickState_On : ChannelChoiceTickState_Off;
}

SvLocal void
QBSatelliteSetupEditorPaneTransponderToggle(void *self_,
                                            QBListPane pane,
                                            SvObject transponder,
                                            int pos)
{
    QBSatelliteSetupEditorPane self = self_;

    if (SvHashTableFind(self->tickedTransponders, transponder))
        SvHashTableRemove(self->tickedTransponders, transponder);
    else
        SvHashTableInsert(self->tickedTransponders, transponder, transponder);
    QBActiveArrayPropagateObjectsChange(self->allTransponders, pos, 1, NULL);
}

SvLocal void
QBSatelliteSetupEditorPaneTransponderAdded(void *self_,
                                           QBTransponderEditPane pane,
                                           QBBasicTransponderData transponderData)
{
    QBSatelliteSetupEditorPane self = self_;

    QBDVBSatelliteTransponder transponder = QBDVBSatelliteTransponderCreate(true, NULL);
    transponder->desc.mux_id.freq = transponderData->freq;
    transponder->desc.symbol_rate = transponderData->symbolRate;
    transponder->desc.polarization = transponderData->polarization;
    transponder->desc.modulation = transponderData->modulation;

    // transponder is required to save satellite, thus this trigger
    bool satelliteAdded = false;
    if (self->newSatellite) {
        QBSatelliteSetupEditorPaneSatelliteEditSaveNew(self);
        satelliteAdded = true;
    }
    int ret = QBDVBSatelliteDescriptorAddTransponder(self->selectedSatellite, transponder);
    if (ret != QBDVBSatelliteTransponderCheck_correct) {
        QBSatelliteSetupEditorPaneTransponderShowPopup(self, ret);
        SVRELEASE(transponder);
        return;
    }

    QBDVBSatellitesDBSaveEntry(self->satellitesDB, self->selectedSatellite);

    // if transponder finished creation of new satellite, notify accordingly
    if (satelliteAdded) {
        ssize_t satIdx = SvArrayIndexOfObject(self->satellitesList, (SvObject) self->selectedSatellite);
        QBSatelliteSetupEditorPaneNotify(self, satIdx, 1, QBSatelliteSetupNotification_ITEM_ADDED);
        QBContextMenuHide(self->contextMenu, false);
    } else {
        //update transponder list before returning from "add transponder" view
        size_t cnt = QBActiveArrayCount(self->allTransponders);
        int idx = QBActiveArrayLowerBound(self->allTransponders, 0, cnt,
                                          (SvObject) transponder,
                                          compareTransponders, NULL);
        QBActiveArrayInsertObjectAtIndex(self->allTransponders, idx, (SvObject) transponder, NULL);

        QBContextMenuPopPane(self->contextMenu);
        QBSatelliteSetupEditorPaneNotifyWithChange(self);
    }

    SVRELEASE(transponder);
}

SvLocal void
QBSatelliteSetupEditorPaneTransponderEdited(void *self_,
                                            QBTransponderEditPane pane,
                                            QBBasicTransponderData transponderData)
{
    QBSatelliteSetupEditorPane self = self_;

    QBDVBSatelliteTransponder transponder = QBDVBSatelliteTransponderCreate(true, NULL);
    transponder->desc.mux_id.freq = transponderData->freq;
    transponder->desc.symbol_rate = transponderData->symbolRate;
    transponder->desc.polarization = transponderData->polarization;
    transponder->desc.modulation = transponderData->modulation;

    int ret = QBDVBSatelliteDescriptorChangeTransponders(self->selectedSatellite,
                                                         self->selectedTransponder,
                                                         transponder);
    if (ret != QBDVBSatelliteTransponderCheck_correct) {
        QBSatelliteSetupEditorPaneTransponderShowPopup(self, ret);
        SVRELEASE(transponder);
        return;
    }

    // decrease by one because of "Add" option
    size_t maxIdx = QBActiveArrayCount(self->allTransponders) - 1;
    // search all existing objects
    ssize_t idx = QBActiveArrayIndexOfObjectFast(self->allTransponders, 0, maxIdx,
                                                 (SvObject) self->selectedTransponder,
                                                 compareTransponders, NULL);
    if (transponder->desc.mux_id.freq == self->selectedTransponder->desc.mux_id.freq) {
        QBActiveArraySetObjectAtIndex(self->allTransponders, idx, (SvObject) transponder, NULL);
    } else {
        QBActiveArrayRemoveObjectAtIndex(self->allTransponders, idx, NULL);
        // allow object larger than those existing, remembering about removed
        size_t idxLowerBound = QBActiveArrayLowerBound(self->allTransponders, 0, maxIdx,
                                                       (SvObject) transponder,
                                                       compareTransponders, NULL);
        QBActiveArrayInsertObjectAtIndex(self->allTransponders, idxLowerBound, (SvObject) transponder, NULL);
    }
    SVRELEASE(transponder);

    QBDVBSatellitesDBSaveEntry(self->satellitesDB, self->selectedSatellite);

    QBSatelliteSetupEditorPaneNotifyWithChange(self);
    QBContextMenuPopPane(self->contextMenu); // pop transponder edit pane
}

SvLocal bool
QBSatelliteSetupEditorPaneEditTranspondersToggleSave(QBSatelliteSetupEditorPane self)
{
    SvIterator it = SvHashTableValuesIterator(self->tickedTransponders);
    QBDVBSatelliteTransponder transponder = NULL;
    int count = 0;
    while ((transponder = (QBDVBSatelliteTransponder) SvIteratorGetNext(&it))) {
        if (SvHashTableFind(self->tickedTransponders, (SvObject) transponder)) {
            count++;
        }
    }
    if (count == 0) {
        self->dialog = QBSatelliteSetupEditorPaneCreatePopup(self,
                                                             gettext("Disabling not possible"),
                                                             gettext("At least one active transponder must exist."));
        QBDialogRun(self->dialog, self, QBSatelliteSetupEditorPaneTransponderPopupCallback);
        return false;
    }
    it = QBActiveArrayIterator(self->allTransponders);
    while ((transponder = (QBDVBSatelliteTransponder) SvIteratorGetNext(&it))) {
        if (SvHashTableFind(self->tickedTransponders, (SvObject) transponder))
            QBDVBSatelliteDescriptorEnableTransponder(self->selectedSatellite, transponder);
        else
            QBDVBSatelliteDescriptorDisableTransponder(self->selectedSatellite, transponder);
    }
    QBDVBSatellitesDBSaveEntry(self->satellitesDB, self->selectedSatellite);

    QBSatelliteSetupEditorPaneNotifyWithChange(self);
    return true;
}

SvLocal bool
QBSatelliteSetupEditorPaneEditTranspondersRemoveSave(QBSatelliteSetupEditorPane self)
{
    size_t cnt = SvHashTableCount(self->tickedTransponders);
    if (cnt == 0)
        return true;

    SvIterator it = QBActiveArrayIterator(self->allTransponders);
    QBDVBSatelliteTransponder transponder = NULL;
    size_t count = 0;
    while ((transponder = (QBDVBSatelliteTransponder) SvIteratorGetNext(&it))) {
        // count "dead" transponders (to be deleted or disabled)
        if (SvHashTableFind(self->tickedTransponders, (SvObject) transponder) || !transponder->isEnabled) {
            count++;
        }
    }
    if (count == QBActiveArrayCount(self->allTransponders)) {
        self->dialog = QBSatelliteSetupEditorPaneCreatePopup(self,
                                                             gettext("Removal not possible"),
                                                             gettext("At least one active transponder must exist."));
        QBDialogRun(self->dialog, self, QBSatelliteSetupEditorPaneTransponderPopupCallback);
        return false;
    }
    it = SvHashTableValuesIterator(self->tickedTransponders);
    while ((transponder = (QBDVBSatelliteTransponder) SvIteratorGetNext(&it))) {
        size_t maxIdx = QBActiveArrayCount(self->allTransponders) - 1;
        ssize_t idx = QBActiveArrayIndexOfObjectFast(self->allTransponders, 0, maxIdx,
                                                     (SvObject) transponder,
                                                     compareTransponders, NULL);
        QBActiveArrayRemoveObjectAtIndex(self->allTransponders, idx, NULL);
        QBDVBSatelliteDescriptorRemoveTransponder(self->selectedSatellite, transponder);
    }

    QBDVBSatellitesDBSaveEntry(self->satellitesDB, self->selectedSatellite);

    ssize_t idx = SvArrayIndexOfObject(self->satellitesList, (SvObject) self->selectedSatellite);
    assert(idx >= 0);
    QBSatelliteSetupEditorPaneNotify(self, idx, 1, QBSatelliteSetupNotification_ITEM_REMOVED);
    return true;
}

SvLocal void
QBSatelliteSetupEditorPaneSingleTransponderSelected(void *self_,
                                                    QBListPane pane,
                                                    SvObject obj,
                                                    int pos)
{
    QBSatelliteSetupEditorPane self = self_;

    int level = QBListPaneGetLevel(pane);

    if (!SvObjectIsInstanceOf(obj, QBDVBSatelliteTransponder_getType()))
        return;

    QBDVBSatelliteTransponder transponder = (QBDVBSatelliteTransponder) obj;
    SVRETAIN(transponder);
    SVTESTRELEASE(self->selectedTransponder);
    self->selectedTransponder = transponder;
    self->selectedTransponderPosition = pos;
    QBSatelliteSetupEditorPaneTransponderShowEditor(self, level, QBSatelliteSetupEditorPaneTransponderEdited, self->selectedTransponder);
}

SvLocal void
QBSatelliteSetupEditorPaneCreateTranspondersList(QBSatelliteSetupEditorPane self, int level)
{
    static struct QBChannelChoiceControllerCallbacks_t callbacks = {
        .isTicked          = QBSatelliteSetupEditorPaneTransponderIsTicked,
        .createNodeCaption = QBSatelliteSetupEditorPaneCreateTransponderCaption
    };

    static struct QBListPaneCallbacks_t listCallbacks = {
        .selected = QBSatelliteSetupEditorPaneTransponderToggle
    };

    SvArray transponders = QBDVBSatelliteDescriptorGetTransponders(self->selectedSatellite);
    SvIterator it = SvArrayIterator(transponders);
    SVTESTRELEASE(self->allTransponders);
    self->allTransponders = QBActiveArrayCreateWithValues(&it, NULL);

    QBListPane transpondersPane = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));
    QBChannelChoiceController constructor =
        QBChannelChoiceControllerCreateFromSettings("ChannelChoiceMenu.settings",
                                                    textRenderer,
                                                    (SvObject) self->allTransponders,
                                                    self, &callbacks);

    svSettingsRestoreContext(self->settingsCtx);
    QBListPaneInit(transpondersPane, self->app, (SvObject) self->allTransponders,
                   (SvObject) constructor, &listCallbacks, self, self->contextMenu,
                   level, SVSTRING("ListPane"));
    svSettingsPopComponent();

    SVRELEASE(constructor);

    self->transpondersSelectionPane = transpondersPane;
    QBContextMenuPushPane(self->contextMenu, (SvObject) transpondersPane);
    SVRELEASE(transpondersPane);
}

SvLocal void
QBSatelliteSetupEditorPaneCreateTranspondersListForEdit(QBSatelliteSetupEditorPane self, int level)
{
    static struct QBChannelChoiceControllerCallbacks_t callbacks = {
        .isTicked          = QBSatelliteSetupEditorPaneTransponderSingleEditIsTicked,
        .createNodeCaption = QBSatelliteSetupEditorPaneCreateTransponderCaption
    };

    static struct QBListPaneCallbacks_t listCallbacks = {
        .selected = QBSatelliteSetupEditorPaneSingleTransponderSelected
    };

    SvArray transponders = QBDVBSatelliteDescriptorGetTransponders(self->selectedSatellite);
    SvIterator it = SvArrayIterator(transponders);
    SVTESTRELEASE(self->allTransponders);
    self->allTransponders = QBActiveArrayCreateWithValues(&it, NULL);

    QBListPane transpondersPane = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));
    QBChannelChoiceController constructor =
        QBChannelChoiceControllerCreateFromSettings("ChannelChoiceMenu.settings",
                                                    textRenderer,
                                                    (SvObject) self->allTransponders,
                                                    self, &callbacks);

    svSettingsRestoreContext(self->settingsCtx);
    QBListPaneInit(transpondersPane, self->app, (SvObject) self->allTransponders,
                   (SvObject) constructor, &listCallbacks, self, self->contextMenu,
                   level, SVSTRING("ListPane"));
    svSettingsPopComponent();

    SVRELEASE(constructor);

    QBContextMenuPushPane(self->contextMenu, (SvObject) transpondersPane);
    SVRELEASE(transpondersPane);
}

SvLocal void
QBSatelliteSetupEditorPaneDefaultSatellite(QBSatelliteSetupEditorPane self)
{
    QBDVBSatellitesDBRestoreSatellite(self->satellitesDB, self->selectedSatellite);
    ssize_t idx = SvArrayIndexOfObject(self->satellitesList, (SvObject) self->selectedSatellite);
    QBSatelliteSetupEditorPaneNotify(self, idx, 1, QBSatelliteSetupNotification_ITEM_CHANGED);
}

SvLocal void
QBSatelliteSetupEditorPaneRemove(QBSatelliteSetupEditorPane self)
{
    ssize_t idx = SvArrayIndexOfObject(self->satellitesList, (SvObject) self->selectedSatellite);
    assert(idx >= 0);

    if (SvArrayCount(self->satellitesList) <= 1) {
        SvWidget lastSatellitePopup =
            QBSatelliteSetupEditorPaneCreatePopup(self, gettext("Action prohibited"),
                                                  gettext("Cannot remove last satellite.\n"));
        QBDialogRun(lastSatellitePopup, self, NULL);
        return;
    }

    SvArrayRemoveObjectAtIndex(self->satellitesList, idx);
    SvString satelliteID = QBDVBSatelliteDescriptorGetID(self->selectedSatellite);
    QBTunerLNBConfig cfg = QBTunerGetLNBConfigByID(satelliteID);
    if (cfg)
        QBTunerLNBConfigRemove(cfg, NULL);

    QBDVBSatellitesDBRemoveEntry(self->satellitesDB, self->selectedSatellite);

    QBSatelliteSetupEditorPaneNotify(self, idx, 1, QBSatelliteSetupNotification_ITEM_REMOVED);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal bool
QBSatelliteSetupEditorPaneCanModifySelectedSatellite(QBSatelliteSetupEditorPane self)
{
    SvString satelliteID = QBDVBSatelliteDescriptorGetID(self->selectedSatellite);
    if (!QBSatelliteSetupLogicCanModifyDefaultSatellites()
        && satelliteID
        && QBDVBSatellitesDBIsDefaultSatellite(self->satellitesDB, satelliteID)) {
        return false;
    }
    return true;
}

SvLocal void
QBSatelliteSetupEditorPaneShowNonRemovableSatelliteDialog(QBSatelliteSetupEditorPane self)
{
    const char *title = gettext("Default satellite");
    const char *message = gettext("You can not remove nor modify default satellite.");
    self->dialog = QBSatelliteSetupEditorPaneCreatePopup(self, title, message);
    QBDialogRun(self->dialog, self, QBSatelliteSetupEditorPaneTransponderPopupCallback);
}

SvLocal bool
QBSatelliteSetupEditorPaneItemSelected(SvObject self_,
                                       SvObject node_,
                                       SvObject nodePath,
                                       QBBasicPane pane,
                                       QBBasicPaneItem item)
{
    QBSatelliteSetupEditorPane self = (QBSatelliteSetupEditorPane) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString id = QBActiveTreeNodeGetID(node);

    if (SvStringEqualToCString(id, "AddSatellite")) {
        svSettingsRestoreContext(self->settingsCtx);
        QBSatelliteEditPane edit =
            QBSatelliteEditPaneCreate(self->contextMenu, self->app,
                                      QBBasicPaneGetLevel(pane) + 1,
                                      NULL, self->satellitesList,
                                      QBSatelliteSetupEditorPaneSatelliteAdded,
                                      self, NULL);
        svSettingsPopComponent();
        QBContextMenuPushPane(self->contextMenu, (SvObject) edit);
        SVRELEASE(edit);

        return true;
    }
    if (SvStringEqualToCString(id, "RemoveSatellite.OK")) {
        if (QBSatelliteSetupEditorPaneCanModifySelectedSatellite(self)) {
            QBSatelliteSetupEditorPaneRemove(self);
        } else {
            QBSatelliteSetupEditorPaneShowNonRemovableSatelliteDialog(self);
            QBContextMenuPopPane(self->contextMenu);
        }
        return true;
    }
    if (SvStringEqualToCString(id, "RemoveSatellite.Cancel")) {
        QBContextMenuPopPane(self->contextMenu);

        return true;
    }
    if (SvStringEqualToCString(id, "EditSatellite")) {
        if (!QBSatelliteSetupEditorPaneCanModifySelectedSatellite(self)) {
            QBSatelliteSetupEditorPaneShowNonRemovableSatelliteDialog(self);
            QBContextMenuPopPane(self->contextMenu);
            return true;
        }

        svSettingsRestoreContext(self->settingsCtx);
        QBSatelliteEditPane edit =
            QBSatelliteEditPaneCreate(self->contextMenu, self->app,
                                      QBBasicPaneGetLevel(pane) + 1,
                                      self->selectedSatellite, self->satellitesList,
                                      QBSatelliteSetupEditorPaneSatelliteEdited,
                                      self, NULL);
        svSettingsPopComponent();
        QBContextMenuPushPane(self->contextMenu, (SvObject) edit);
        SVRELEASE(edit);

        return true;
    }
    if (SvStringEqualToCString(id, "SortSatellites")) {
        unsigned int level = QBBasicPaneGetLevel(pane);
        QBSatelliteSetupEditorPaneSatelliteSortShowPane(self, level + 1);

        return true;
    }
    if (SvStringEqualToCString(id, "EditTransponders.Add")) {
        if (!QBSatelliteSetupEditorPaneCanModifySelectedSatellite(self)) {
            QBSatelliteSetupEditorPaneShowNonRemovableSatelliteDialog(self);
            QBContextMenuPopPane(self->contextMenu);
            return true;
        }

        unsigned int level = QBBasicPaneGetLevel(pane);
        QBSatelliteSetupEditorPaneTransponderShowEditor(self, level, QBSatelliteSetupEditorPaneTransponderAdded, NULL);

        return true;
    }
    if (SvStringEqualToCString(id, "EditTransponders.Edit")) {
        if (!QBSatelliteSetupEditorPaneCanModifySelectedSatellite(self)) {
            QBSatelliteSetupEditorPaneShowNonRemovableSatelliteDialog(self);
            QBContextMenuPopPane(self->contextMenu);
            return true;
        }

        unsigned int level = QBBasicPaneGetLevel(pane);
        QBSatelliteSetupEditorPaneCreateTranspondersListForEdit(self, level + 1);

        return true;
    }
    if (SvStringEqualToCString(id, "EditTransponders.Toggle")) {
        SvArray transponders = QBDVBSatelliteDescriptorGetTransponders(self->selectedSatellite);
        size_t cnt = SvArrayCount(transponders);
        if (cnt == 0)
            return true;

        SVTESTRELEASE(self->tickedTransponders);
        self->tickedTransponders = SvHashTableCreate(197, NULL);

        SvIterator it = SvArrayIterator(transponders);
        QBDVBSatelliteTransponder transponder = NULL;
        while ((transponder = (QBDVBSatelliteTransponder) SvIteratorGetNext(&it))) {
            if (transponder->isEnabled)
                SvHashTableInsert(self->tickedTransponders, (SvObject) transponder,
                                  (SvObject) transponder);
        }

        self->confirmationType = QBSatelliteSetupEditorPaneConfirmation_TOOGLE_TRANSPONDERS;
        QBSatelliteSetupEditorPaneCreateTranspondersList(self, QBBasicPaneGetLevel(pane) + 1);

        return true;
    }
    if (SvStringEqualToCString(id, "EditTransponders.Remove")) {
        if (!QBSatelliteSetupEditorPaneCanModifySelectedSatellite(self)) {
            QBSatelliteSetupEditorPaneShowNonRemovableSatelliteDialog(self);
            QBContextMenuPopPane(self->contextMenu);
            return true;
        }

        SvArray transponders = QBDVBSatelliteDescriptorGetTransponders(self->selectedSatellite);
        size_t cnt = SvArrayCount(transponders);
        if (cnt == 0)
            return true;

        SVTESTRELEASE(self->tickedTransponders);
        self->tickedTransponders = SvHashTableCreate(197, NULL);

        self->confirmationType = QBSatelliteSetupEditorPaneConfirmation_REMOVE_TRANSPONDERS;
        QBSatelliteSetupEditorPaneCreateTranspondersList(self, QBBasicPaneGetLevel(pane) + 1);

        return true;
    }
    if (SvStringEqualToCString(id, "RestoreSatellite.OK")) {
        QBSatelliteSetupEditorPaneDefaultSatellite(self);
        QBContextMenuHide(self->contextMenu, false);
        return true;
    }
    if (SvStringEqualToCString(id, "RestoreSatellite.Cancel")) {
        QBContextMenuPopPane(self->contextMenu);
        return true;
    }

    return false;
}

SvLocal void
QBSatelliteSetupEditorPaneShow(SvObject self_)
{
    QBSatelliteSetupEditorPane self = (QBSatelliteSetupEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, show);
}

SvLocal void
QBSatelliteSetupEditorPaneHide(SvObject self_, bool immediately)
{
    QBSatelliteSetupEditorPane self = (QBSatelliteSetupEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, hide, immediately);
}

SvLocal void
QBSatelliteSetupEditorPaneSetActive(SvObject self_)
{
    QBSatelliteSetupEditorPane self = (QBSatelliteSetupEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, setActive);
}

SvLocal bool
QBSatelliteSetupEditorPaneHandleInputEvent(SvObject self_,
                                           SvObject src,
                                           SvInputEvent e)
{
    return false;
}

SvLocal void
QBSatelliteSetupEditorPane__dtor__(void *ptr)
{
    QBSatelliteSetupEditorPane self = ptr;
    QBContextMenuSetInputEventHandler(self->contextMenu, NULL, NULL);

    SVRELEASE(self->options);
    SVRELEASE(self->satellitesList);
    SVTESTRELEASE(self->listener);
    SVTESTRELEASE(self->tickedTransponders);
    SVTESTRELEASE(self->allTransponders);
    SVTESTRELEASE(self->selectedTransponder);
    SVTESTRELEASE(self->satellitesDB);
}

SvLocal SvType
QBSatelliteSetupEditorPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSatelliteSetupEditorPane__dtor__
    };
    static SvType type = NULL;

    static const struct QBAutoMenuEventHandler_ menuHandler = {
        .itemSelected = QBSatelliteSetupEditorPaneItemSelected,
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBSatelliteSetupEditorPaneShow,
        .hide             = QBSatelliteSetupEditorPaneHide,
        .setActive        = QBSatelliteSetupEditorPaneSetActive,
        .handleInputEvent = QBSatelliteSetupEditorPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSatelliteSetupEditorPane",
                            sizeof(struct QBSatelliteSetupEditorPane_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBAutoMenuEventHandler_getInterface(), &menuHandler,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }
    return type;
}

SvLocal void
QBSatelliteSetupEditorPaneSatelliteEditSave(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBSatelliteSetupEditorPane self = self_;
    bool state = false;

    if (self->confirmationType == QBSatelliteSetupEditorPaneConfirmation_TOOGLE_TRANSPONDERS) {
        state = QBSatelliteSetupEditorPaneEditTranspondersToggleSave(self);
    } else if (QBSatelliteSetupEditorPaneConfirmation_REMOVE_TRANSPONDERS) {
        state = QBSatelliteSetupEditorPaneEditTranspondersRemoveSave(self);
    }

    QBContextMenuPopPane(self->contextMenu); // Pop confirmation pane
    if (state) {
        QBContextMenuPopPane(self->contextMenu); // Pop transponders list pane
    }
}

SvLocal void
QBSatelliteSetupEditorPaneSatelliteEditCancel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBSatelliteSetupEditorPane self = self_;

    QBContextMenuPopPane(self->contextMenu); // Pop confirmation pane
    QBContextMenuPopPane(self->contextMenu); // Pop transponders list pane
}

SvLocal void
QBSatelliteSetupEditorPaneSatelliteEditConfirm(QBSatelliteSetupEditorPane self, unsigned int level)
{
    svSettingsRestoreContext(self->settingsCtx);
    QBBasicPane confirmation = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));
    QBBasicPaneInit(confirmation, self->app,
                    SvSchedulerGet(), textRenderer,
                    self->contextMenu, level, SVSTRING("BasicPane"));
    svSettingsPopComponent();

    SvString option = SvStringCreate(gettext("Save"), NULL);
    QBBasicPaneAddOption(confirmation, SVSTRING("save"), option,
                         QBSatelliteSetupEditorPaneSatelliteEditSave, self);
    SVRELEASE(option);

    option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(confirmation, SVSTRING("cancel"), option,
                         QBSatelliteSetupEditorPaneSatelliteEditCancel, self);
    SVRELEASE(option);

    QBContextMenuPushPane(self->contextMenu, (SvObject) confirmation);
    SVRELEASE(confirmation);
}

SvLocal bool
QBSatelliteSetupEditorPaneGlobalInputEventHandler(void *self_, SvObject src, SvInputEvent e)
{
    QBSatelliteSetupEditorPane self = self_;

    if (src == (SvObject) self->transpondersSelectionPane) {
        int level = QBListPaneGetLevel(self->transpondersSelectionPane);
        if (e->ch == QBKEY_LEFT || e->ch == QBKEY_RIGHT) {
            QBSatelliteSetupEditorPaneSatelliteEditConfirm(self, level + 1);
            return true;
        }
    }
    return false;
}

SvLocal void
QBSatelliteSetupEditorPaneInit(QBSatelliteSetupEditorPane self,
                               SvApplication app,
                               QBDVBSatellitesDB satellitesDB,
                               QBContextMenu ctxMenu,
                               SvString widgetName,
                               SvArray satelliteList,
                               int selectedSatellite,
                               bool singleSatelliteMode)
{
    self->app = app;
    self->satellitesDB = SVRETAIN(satellitesDB);
    self->contextMenu = ctxMenu;

    self->satellitesList = SVRETAIN(satelliteList);
    self->selectedSatellite = (QBDVBSatelliteDescriptor) SvArrayAt(self->satellitesList, selectedSatellite);

    self->settingsCtx = svSettingsSaveContext();

    SvString menuDescriptionFile;
    if (singleSatelliteMode)
        menuDescriptionFile = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/SatelliteMenuSingle.xml", SvGetRuntimePrefix());
    else
        menuDescriptionFile = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/SatelliteMenu.xml", SvGetRuntimePrefix());
    self->options = QBAutoMenuCreate(app, ctxMenu, menuDescriptionFile,
                                     (SvObject) self, NULL);
    SVRELEASE(menuDescriptionFile);


    QBContextMenuSetInputEventHandler(self->contextMenu, self, QBSatelliteSetupEditorPaneGlobalInputEventHandler);
}

int
QBSatelliteSetupEditorPaneSetListener(QBSatelliteSetupEditorPane self,
                                      SvObject listener)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    }
    if (!listener) {
        SvLogError("%s: NULL listener passed", __func__);
        return -1;
    }

    self->listener = SvWeakReferenceCreateWithObject(listener, NULL);
    return 0;
}


QBSatelliteSetupEditorPane
QBSatelliteSetupEditorPaneCreateFromSettings(const char *settings,
                                             SvApplication app,
                                             QBContextMenu ctxMenu,
                                             SvString widgetName,
                                             SvArray satellitesList,
                                             int selectedSatellite,
                                             bool singleSatelliteMode,
                                             SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBSatelliteSetupEditorPane self = NULL;

    if (!settings || !app || !ctxMenu
        || !widgetName || !satellitesList) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed :\n"
                                  "\tsettings          = %s,\n"
                                  "\tapp               = %p,\n"
                                  "\tctxMenu           = %p,\n"
                                  "\twidgetName        = %s,\n"
                                  "\tsatellitesList    = %p",
                                  settings, app, ctxMenu,
                                  widgetName ? SvStringCString(widgetName) : NULL,
                                  satellitesList);
        goto out;
    }

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), &error);
    if (!satellitesDB) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "QBGlobalStorageGetItem(QBDVBSatellitesDB_getType()) failed");
        goto out;
    }

    self = (QBSatelliteSetupEditorPane) SvTypeAllocateInstance(QBSatelliteSetupEditorPane_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBSateliteSetupEditorPane");
        goto out;
    }

    svSettingsPushComponent(settings);

    QBSatelliteSetupEditorPaneInit(self, app, satellitesDB, ctxMenu, widgetName,
                                   satellitesList, selectedSatellite, singleSatelliteMode);
    svSettingsPopComponent();

out:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

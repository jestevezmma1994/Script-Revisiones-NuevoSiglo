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

#include "QBRecordingIcon.h"

#include <settings.h>
#include <Services/pvrAgent.h>
#include <QBPVRRecording.h>
#include <NPvr/QBnPVRProvider.h>
#include <NPvr/QBnPVRRecording.h>
#include <Utils/QBEventUtils.h>
#include <Logic/NPvrLogic.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <CUIT/Core/widget.h>

#define log_debug(fmt, ...)  do { if(0) SvLogNotice(COLBEG() "%s:%d " fmt  COLEND_COL(blue), __func__,__LINE__,##__VA_ARGS__); } while (0)


struct QBRecordingIconUpdater_t {
    struct SvObject_ super_;
    SvWeakList queue, all;
    SvFiber fiber;
    bool started;
    QBPVRProvider pvrProvider;
    SvObject nPVRProvider;
};

typedef struct QBRecordingIconUpdater_t *QBRecordingIconUpdater;
QBRecordingIconUpdater updater = NULL;

SvLocal void
QBRecordingIconScheduleUpdate(QBRecordingIcon self);

struct QBRecordingIconConstructor_t {
    struct SvObject_ super_;
    SvBitmap recordingTagFinished,
             recordingTagFuture,
             recordingTagNow,
             recordingTagKeyword,
             recordingTagRepeated,
             recordingTagCapable,
             recordingTagPartial;
    int recX, recY;
    int width, height;
};

SvLocal void
QBRecordingIconConstructor__dtor__(void *self_)
{
    QBRecordingIconConstructor self = self_;
    SVTESTRELEASE(self->recordingTagFinished);
    SVTESTRELEASE(self->recordingTagNow);
    SVTESTRELEASE(self->recordingTagFuture);
    SVTESTRELEASE(self->recordingTagCapable);
    SVTESTRELEASE(self->recordingTagKeyword);
    SVTESTRELEASE(self->recordingTagRepeated);
    SVTESTRELEASE(self->recordingTagPartial);
}

SvLocal SvType
QBRecordingIconConstructor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRecordingIconConstructor__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRecordingIconConstructor",
                            sizeof(struct QBRecordingIconConstructor_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

QBRecordingIconConstructor
QBRecordingIconConstructorNew(const char *name)
{
    QBRecordingIconConstructor self = (QBRecordingIconConstructor)SvTypeAllocateInstance(QBRecordingIconConstructor_getType(), NULL);
    self->recordingTagFinished  = SVTESTRETAIN(svSettingsGetBitmap(name, "tagFinished"));
    self->recordingTagFuture    = SVTESTRETAIN(svSettingsGetBitmap(name, "tagFuture"));
    self->recordingTagNow       = SVTESTRETAIN(svSettingsGetBitmap(name, "tagNow"));
    self->recordingTagKeyword   = SVTESTRETAIN(svSettingsGetBitmap(name, "tagKeyword"));
    self->recordingTagRepeated  = SVTESTRETAIN(svSettingsGetBitmap(name, "tagRepeated"));
    self->recordingTagCapable   = SVTESTRETAIN(svSettingsGetBitmap(name, "tagCapable"));
    self->recordingTagPartial   = SVTESTRETAIN(svSettingsGetBitmap(name, "tagPartial"));

    self->recX = svSettingsGetInteger(name, "recXOffset", 0);
    self->recY = svSettingsGetInteger(name, "recYOffset", 0);
    self->width = svSettingsGetInteger(name, "width", 0);
    self->height = svSettingsGetInteger(name, "height", 0);
    return self;
}

struct QBRecordingIcon_t {
    struct SvObject_ super_;

    SvBitmap recordingTagFinished,
             recordingTagFuture,
             recordingTagNow,
             recordingTagKeyword,
             recordingTagRepeated,
             recordingTagCapable,
             recordingTagPartial;
    SvWidget recIcon;
    SvEPGEvent event;
    SvWidget w;
    QBPVRProvider pvrProvider;
    SvObject nPVRProvider;
};

SvLocal void
QBRecordingIconUpdateAll(QBRecordingIconUpdater self)
{
    SvWeakListRemoveAllObjects(self->queue);
    SvIterator it = SvWeakListIterator(self->all);
    SvGenericObject obj = NULL;
    while ((obj = SvIteratorGetNext(&it))) {
        SvWeakListPushBack(self->queue, obj, NULL);
    }
    SvFiberActivate(self->fiber);
}

SvLocal void
QBRecordingIconPVRRecordingAddedRemoved(SvGenericObject prv_, QBPVRRecording recording)
{
    QBRecordingIconUpdateAll((QBRecordingIconUpdater) prv_);
}

SvLocal void
QBRecordingIconPVRRecordingRestricted(SvObject self_,
                                      QBPVRRecording rec)
{
}

SvLocal void
QBRecordingIconPVRRecordingChanged(SvGenericObject prv_, QBPVRRecording recording, QBPVRRecording oldRecording)
{
    QBRecordingIconUpdateAll((QBRecordingIconUpdater) prv_);
}

SvLocal void
QBRecordingIconNPvrRecordingChanged(SvGenericObject prv_, QBnPVRRecording recording)
{
    QBRecordingIconUpdateAll((QBRecordingIconUpdater) prv_);
}

SvLocal void
QBRecordingIconPVRQuotaChanged(SvObject self, QBPVRQuota quota)
{
}

SvLocal void
QBRecordingIconPVRDirectoryAdded(SvObject self_, QBPVRDirectory directory)
{
    QBPVRDirectoryAddListener(directory, self_);
    QBRecordingIconUpdateAll((QBRecordingIconUpdater) self_);
}

SvLocal void
QBRecordingIconPVRDirectoryChanged(SvObject self_, QBPVRDirectory directory)
{
    QBRecordingIconUpdateAll((QBRecordingIconUpdater) self_);
}

SvLocal void QBRecordingIcon__dtor__(void *self_)
{
    QBRecordingIcon prv = self_;

    SVTESTRELEASE(prv->recordingTagFinished);
    SVTESTRELEASE(prv->recordingTagFuture);
    SVTESTRELEASE(prv->recordingTagNow);
    SVTESTRELEASE(prv->recordingTagKeyword);
    SVTESTRELEASE(prv->recordingTagRepeated);
    SVTESTRELEASE(prv->recordingTagCapable);
    SVTESTRELEASE(prv->recordingTagPartial);
    SVTESTRELEASE(prv->event);
    SVTESTRELEASE(prv->pvrProvider);
    SVTESTRELEASE(prv->nPVRProvider);
}

SvLocal SvType
QBRecordingIcon_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRecordingIcon__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRecordingIcon",
                            sizeof(struct QBRecordingIcon_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void
QBRecordingIconClean(SvApplication app, void *prv_)
{
    QBRecordingIcon prv = prv_;
    SVRELEASE(prv);
}
SvWidget
QBRecordingIconNewFromConstructor(SvApplication app,
                                  QBPVRProvider pvrProvider,
                                  SvObject nPVRProvider,
                                  QBRecordingIconConstructor constructor)
{
    QBRecordingIcon prv = (QBRecordingIcon)SvTypeAllocateInstance(QBRecordingIcon_getType(), NULL);

    prv->pvrProvider = SVTESTRETAIN(pvrProvider);
    prv->nPVRProvider = SVTESTRETAIN(nPVRProvider);
    prv->recordingTagFinished = SVTESTRETAIN(constructor->recordingTagFinished);
    prv->recordingTagNow      = SVTESTRETAIN(constructor->recordingTagNow);
    prv->recordingTagFuture   = SVTESTRETAIN(constructor->recordingTagFuture);
    prv->recordingTagKeyword  = SVTESTRETAIN(constructor->recordingTagKeyword);
    prv->recordingTagRepeated = SVTESTRETAIN(constructor->recordingTagRepeated);
    prv->recordingTagCapable  = SVTESTRETAIN(constructor->recordingTagCapable);
    prv->recordingTagPartial  = SVTESTRETAIN(constructor->recordingTagPartial);

    SvWidget w = svWidgetCreateBitmap(app, constructor->width, constructor->height, NULL);
    w->prv = prv;
    w->clean = QBRecordingIconClean;

    prv->recIcon = svWidgetCreateBitmap(app, 0, 0, NULL);
    svWidgetAttach(w, prv->recIcon, constructor->recX, constructor->recY, 1);

    QBRecordingIconScheduleUpdate(prv);
    SvWeakListPushBack(updater->all, (SvGenericObject)prv, NULL);
    prv->w = w;

    return w;
}

void
QBRecordingIconSetByEPGEvent(SvWidget widget, SvEPGEvent event)
{
    QBRecordingIcon prv = widget->prv;
    SVTESTRELEASE(prv->event);
    prv->event = SVTESTRETAIN(event);
    if(!event) {
        svWidgetSetBitmap(widget, NULL);
        return;
    }
    QBRecordingIconScheduleUpdate(prv);
}

SvLocal void
QBRecordingIconSetByEPGEvent_(SvWidget widget, SvEPGEvent event)
{
    QBRecordingIcon prv = widget->prv;

    QBLangPreferences langPreferences = (QBLangPreferences)
        QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBLangPreferences_getType(), NULL);
    log_debug("Update [%s]", SvStringCString(QBEventUtilsGetRawTitleFromEvent(langPreferences, event)));

    /// PVR provider
    bool completed = false, active = false, future = false, keyword = false, series = false, repeated = false, partial = false;
    if (prv->pvrProvider) {
        QBPVRProviderGetRecordingsParams params = {
            .includeRecordingsInDirectories = true,
            .playable = true,
        };
        params.inStates[QBPVRRecordingState_active] = true;
        params.inStates[QBPVRRecordingState_scheduled] = true;

        SvArray recordings = QBPVRProviderFindRecordingsInTimeRange(prv->pvrProvider,
                                                                    SvValueGetString(event->channelID),
                                                                    SvTimeConstruct(event->startTime, 0),
                                                                    SvTimeConstruct(event->endTime, 0),
                                                                    params);

        SvIterator it = SvArrayIterator(recordings);
        QBPVRRecording recording;
        while((recording = (QBPVRRecording) SvIteratorGetNext(&it))) {
            switch (recording->state) {
                case QBPVRRecordingState_scheduled:
                    future = true;
                    break;
                case QBPVRRecordingState_active:
                    active = true;
                    break;
                default:
                    break;
            }
            if (!active && recording->playable)
                completed = true;
            if (recording->playable && (recording->state != QBPVRRecordingState_completed))
                partial = true;
        }
        SVTESTRELEASE(recordings);

        SvArray dirs = QBPVRProviderCreateActiveDirectoriesListForEvent(prv->pvrProvider, event);
        it = SvArrayGetIterator(dirs);
        QBPVRDirectory dir;
        while ((dir = (QBPVRDirectory) SvIteratorGetNext(&it))) {
            switch (dir->type) {
                case QBPVRDirectoryType_keyword:
                    keyword = true;
                    break;
                case QBPVRDirectoryType_series:
                    series = true;
                    break;
                case QBPVRDirectoryType_repeated:
                    repeated = true;
                    break;
                default:
                    break;
            }
        }
        SVRELEASE(dirs);
    }

    /// NPvrManager
    if (prv->nPVRProvider && SvEPGEventCanBeRemotelyRecorded(event)) {
        SvArray recordings = SvArrayCreate(NULL);
        SvInvokeInterface(QBnPVRProvider, prv->nPVRProvider, listRecordingsByEvent, event, recordings);
        QBnPVRRecording npvrRec;
        SvIterator it = SvArrayIterator(recordings);
        while ((npvrRec = (QBnPVRRecording) SvIteratorGetNext(&it))) {
            if (npvrRec->state == QBnPVRRecordingState_scheduled) {
                future = true;
            } else if (npvrRec->state == QBnPVRRecordingState_active) {
                active = true;
            } else if (npvrRec->state == QBnPVRRecordingState_completed) {
                completed = true;
            }
        }
        SVRELEASE(recordings);
        QBnPVRRecordingType npvrRecType = QBNPvrLogicGetRecordingTypeForEvent(prv->nPVRProvider, event);

        if (npvrRecType == QBnPVRRecordingType_keyword) {
            keyword = true;
        }
        if (npvrRecType == QBnPVRRecordingType_series) {
            series = true;
        }
    }

    /// Apply as widgets
    SvWidget rec = prv->recIcon;
    if (active && prv->recordingTagNow) {
        svWidgetSetBitmap(rec, prv->recordingTagNow);
    } else if (partial && prv->recordingTagPartial) {
        svWidgetSetBitmap(rec, prv->recordingTagPartial);
    } else if (completed) {
        svWidgetSetBitmap(rec, prv->recordingTagFinished);
    } else if (keyword && prv->recordingTagKeyword) {
        svWidgetSetBitmap(rec, prv->recordingTagKeyword);
    } else if ((series || repeated) && prv->recordingTagRepeated) {
        svWidgetSetBitmap(rec, prv->recordingTagRepeated);
    } else if (future && prv->recordingTagFuture) {
        svWidgetSetBitmap(rec, prv->recordingTagFuture);
    } else {
        svWidgetSetBitmap(rec, NULL);
    }

    if (rec->bmp) {
        rec->width = rec->bmp->width;
        rec->height = rec->bmp->height;
    }
}

SvLocal void
QBRecordingIconUpdate__dtor__(void *self_)
{
    QBRecordingIconUpdater self = (QBRecordingIconUpdater) self_;

    if (self->pvrProvider) {
        SvArray dirs = QBPVRProviderCreateDirectoriesList(self->pvrProvider);
        SvIterator it = SvArrayGetIterator(dirs);
        QBPVRDirectory dir;
        while ((dir = (QBPVRDirectory) SvIteratorGetNext(&it)))
            QBPVRDirectoryRemoveListener(dir, (SvObject) self);
        SVRELEASE(dirs);
        SVRELEASE(self->pvrProvider);
    }

    SvFiberDestroy(self->fiber);
    SVRELEASE(self->queue);
    SVRELEASE(self->all);
    SVTESTRELEASE(self->nPVRProvider);
}


SvLocal SvType
QBRecordingIconUpdater_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRecordingIconUpdate__dtor__
    };

    static const struct QBPVRProviderListener_ pvrProviderListenerMethods = {
        .recordingAdded = QBRecordingIconPVRRecordingAddedRemoved,
        .recordingChanged = QBRecordingIconPVRRecordingChanged,
        .recordingRemoved = QBRecordingIconPVRRecordingAddedRemoved,
        .recordingRestricted = QBRecordingIconPVRRecordingRestricted,
        .quotaChanged = QBRecordingIconPVRQuotaChanged,
        .directoryAdded = QBRecordingIconPVRDirectoryAdded,
    };

    static const struct QBnPVRProviderRecordingListener_ nPVRRecordingMethods = {
        .added = QBRecordingIconNPvrRecordingChanged,
        .changed = QBRecordingIconNPvrRecordingChanged,
        .removed = QBRecordingIconNPvrRecordingChanged
    };

    static const struct QBPVRDirectoryListener_ pvrDirectoryListenerMethods = {
        .changed = QBRecordingIconPVRDirectoryChanged,
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRecordingIconUpdater",
                            sizeof(struct QBRecordingIconUpdater_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPVRProviderListener_getInterface(), &pvrProviderListenerMethods,
                            QBnPVRProviderRecordingListener_getInterface(), &nPVRRecordingMethods,
                            QBPVRDirectoryListener_getInterface(),  &pvrDirectoryListenerMethods,
                            NULL);
    }
    return type;
}

SvLocal void QBRecordingIconUpdateStep(void *self_)
{
    QBRecordingIconUpdater self = self_;
    QBRecordingIcon icon;

    while ((icon = (QBRecordingIcon) SvWeakListTakeFirst(self->queue))) {
        QBRecordingIconSetByEPGEvent_(icon->w, icon->event);
        SVRELEASE(icon);
        if (SvFiberTimePassed())
            return;
    }
    SvFiberDeactivate(self->fiber);
}


SvLocal QBRecordingIconUpdater
QBRecordingIconUpdaterCreate(QBPVRProvider pvrProvider, SvObject nPVRProvider)
{
    QBRecordingIconUpdater self = (QBRecordingIconUpdater)SvTypeAllocateInstance(QBRecordingIconUpdater_getType(), NULL);
    self->queue = SvWeakListCreate(NULL);
    self->all = SvWeakListCreate(NULL);
    self->fiber = SvFiberCreate(SvSchedulerGet(), NULL, "QBRecordingIconUpdate", QBRecordingIconUpdateStep, self);
    self->nPVRProvider = SVTESTRETAIN(nPVRProvider);

    if (pvrProvider) {
        self->pvrProvider = SVRETAIN(pvrProvider);
        SvArray dirs = QBPVRProviderCreateDirectoriesList(pvrProvider);
        SvIterator it = SvArrayGetIterator(dirs);
        QBPVRDirectory dir;
        while ((dir = (QBPVRDirectory) SvIteratorGetNext(&it)))
            QBPVRDirectoryAddListener(dir, (SvObject) self);
        SVRELEASE(dirs);
    }

    SvFiberSetPriority(self->fiber, 30); //low priority

    return self;
}

SvLocal void
QBRecordingIconScheduleUpdate(QBRecordingIcon icon)
{
    QBLangPreferences langPreferences = (QBLangPreferences)
        QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBLangPreferences_getType(), NULL);
    if (icon->event)
        log_debug("Schedule update [%s]", SvStringCString(QBEventUtilsGetRawTitleFromEvent(langPreferences,
                                                                                           icon->event)));
    if(!updater) {
        updater = QBRecordingIconUpdaterCreate(icon->pvrProvider, icon->nPVRProvider);
        QBRecordingIconUpdaterStart();
    }
    SvWeakListPushFront(updater->queue, (SvGenericObject)icon, NULL);
    SvFiberActivate(updater->fiber);
}

void
QBRecordingIconUpdaterStart(void)
{
    if (!updater || updater->started)
        return;

    if (updater->pvrProvider)
        QBPVRProviderAddListener(updater->pvrProvider, (SvObject) updater);

    if (updater->nPVRProvider)
        SvInvokeInterface(QBnPVRProvider, updater->nPVRProvider, addRecordingListener, (SvGenericObject) updater);

    updater->started = true;
}

void
QBRecordingIconUpdaterStop(void)
{
    if (!updater || !updater->started)
        return;

    if (updater->pvrProvider)
        QBPVRProviderRemoveListener(updater->pvrProvider, (SvObject) updater);

    if (updater->nPVRProvider)
        SvInvokeInterface(QBnPVRProvider, updater->nPVRProvider, removeRecordingListener, (SvGenericObject) updater);

    SVRELEASE(updater);
    updater = NULL;
}

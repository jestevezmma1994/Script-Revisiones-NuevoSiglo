/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBMWConfigHandler.h"

#include <Services/QBMWConfigHandlerPlugins/QBMWConfigHandlerPlugin.h>
#include <Services/QBMWConfigMonitor.h>
#include <QBConf.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <QBAppKit/QBObserver.h>
#include <Logic/VoDLogic.h>

#include <init.h>
#include <main.h>
#include <time.h>
#include <string.h>
#include <Widgets/QBRestartDialog.h>
#include <QBWidgets/QBDialog.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 0, "QBMWConfigLogLevel", "" );

#define log_error(fmt, ...) if (env_log_level() >= 0) { SvLogError( COLBEG() "QBMWConfigHandler:: " fmt COLEND_COL(red), ##__VA_ARGS__); };
#define log_warning(fmt, ...) if (env_log_level() >= 0) { SvLogWarning( COLBEG() "QBMWConfigHandler:: " fmt COLEND_COL(yellow), ##__VA_ARGS__); };
#define log_state(fmt, ...) if (env_log_level() >= 1) { SvLogNotice( COLBEG() "QBMWConfigHandler :: " fmt COLEND_COL(blue), ##__VA_ARGS__); };
#define log_debug(fmt, ...) if (env_log_level() >= 2) { SvLogNotice( COLBEG() "QBMWConfigHandler :: " fmt COLEND_COL(green), ##__VA_ARGS__); };

struct QBMWConfigHandler_
{
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvWeakList listeners;

    bool started;

    SvHashTable plugins;
};

SvLocal void
QBMWConfigHandlerGotResponse(SvGenericObject self_, SvHashTable response);
SvLocal void
QBMWConfigHandlerNoResponse(SvGenericObject self_);
SvLocal void
QBMWConfigHandlerNotify(QBMWConfigHandler self, SvValue message);

SvLocal void QBMWConfigHandler__dtor__(void *self_)
{
    QBMWConfigHandler self = self_;
    SVRELEASE(self->plugins);
    SVRELEASE(self->listeners);
}

void
QBMWConfigHandlerAddListener(QBMWConfigHandler self, SvObject listener)
{
    if (SvObjectIsImplementationOf(listener, QBObserver_getInterface())) {
        SvWeakListPushBack(self->listeners, listener, NULL);
    } else {
        SvLogWarning("%s(): listener %p of type %s doesn't implement QBObserver interface.", __func__, listener, listener ? SvObjectGetTypeName(listener) : "--");
    }
}

void
QBMWConfigHandlerRemoveListener(QBMWConfigHandler self, SvObject listener)
{
    SvWeakListRemoveObject(self->listeners, listener);
}

SvLocal void
QBMWConfigHandlerNotify(QBMWConfigHandler self, SvValue message)
{
    SvIterator iter = SvWeakListIterator(self->listeners);
    SvObject observer;
    while ((observer = SvIteratorGetNext(&iter))) {
        SvInvokeInterface(QBObserver, observer, observedObjectUpdated, (SvObject) self, (SvObject) message);
    }
}

SvLocal SvType
QBMWConfigHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMWConfigHandler__dtor__
    };
    static SvType type = NULL;

    if (!type) {
        static struct QBMWConfigMonitorListener_ qbMWConfigListener = {
            .gotResponse = QBMWConfigHandlerGotResponse,
            .noResponse = QBMWConfigHandlerNoResponse
        };


        SvTypeCreateManaged("QBMWConfigHandler",
                            sizeof(struct QBMWConfigHandler_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMWConfigMonitorListener_getInterface(), &qbMWConfigListener,
                            NULL);
    }

    return type;
}

QBMWConfigHandler
QBMWConfigHandlerCreate(AppGlobals appGlobals)
{
    QBMWConfigHandler self = (QBMWConfigHandler) SvTypeAllocateInstance(QBMWConfigHandler_getType(), NULL);
    self->appGlobals = appGlobals;
    self->started = false;

    self->listeners = SvWeakListCreate(NULL);
    self->plugins = SvHashTableCreate(3, NULL);

    return self;
}

void
QBMWConfigHandlerStart(QBMWConfigHandler self)
{
    if (unlikely(!self)) {
        log_error("%s NULL argument passed", __func__);
        return;
    }

    if (self->started) {
        return;
    }

    QBMWConfigMonitorAddListener(self->appGlobals->qbMWConfigMonitor, (SvGenericObject) self);

    self->started = true;
}

void
QBMWConfigHandlerStop(QBMWConfigHandler self)
{
    if (unlikely(!self)) {
        log_error("%s NULL argument passed", __func__);
        return;
    }

    if (!self->started) {
        return;
    }

    QBMWConfigMonitorRemoveListener(self->appGlobals->qbMWConfigMonitor, (SvGenericObject) self);

    self->started = false;
}

SvLocal void
QBMWConfigHandlerHandleStandbyConfig(QBMWConfigHandler self, SvGenericObject value)
{
    if (!SvObjectIsInstanceOf(value, SvHashTable_getType())) {
        log_warning("%s: invalid answer (standby_config)", __func__);
        return;
    }

    // standby keys validation
    SvHashTable answer = (SvHashTable) value;
    SvIterator it = SvHashTableKeysIterator(answer);
    SvGenericObject keyObj;
    while ((keyObj = SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf(keyObj, SvString_getType())) {
            log_warning("%s: invalid hashtable key type (standby_config)", __func__);
            return;
        }
    }
}

SvLocal void
QBMWConfigHandlerHandleConfig(QBMWConfigHandler self, SvObject value)
{
    if (!SvObjectIsInstanceOf(value, SvHashTable_getType())) {
        log_warning("%s: invalid answer (config)", __func__);
        return;
    }

    SvHashTable answer = (SvHashTable) value;
    SvIterator it = SvHashTableKeysIterator(answer);
    SvGenericObject keyObj;
    while ((keyObj = SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf(keyObj, SvString_getType())) {
            log_warning("%s: invalid hashtable key type (config)", __func__);
            return;
        }

        SvString keyStr = (SvString) keyObj;
        if (SvStringEqualToCString(keyStr, "standby")) {
            log_state("Got standby_config");
            SvObject v = SvHashTableFind(answer, keyObj);
            QBMWConfigHandlerHandleStandbyConfig(self, v);
        }
    }
}

/**
 * Iterate over plugins and set changed and notify attributes.
 *
 * Changed and notify are passed from plugins. Changed is propagated from
 * a given plugin only if it sets notify true.
 */
SvLocal void
QBMWConfigHandlerPassPairToPlugins(QBMWConfigHandler self, SvString key, SvObject value, bool *changed, bool *notify)
{
    SvIterator it = (SvIterator) SvHashTableValuesIterator(self->plugins);
    SvGenericObject plugin = NULL;
    *changed = false;
    *notify = false;
    while ((plugin = SvIteratorGetNext(&it))) {
        bool changed_, notify_;
        SvInvokeInterface(QBMWConfigHandlerPlugin, plugin, handleConfigPair, key, value, &changed_, &notify_);
        if (notify_)
            *changed |= changed_;
        *notify |= notify_;
    }
}

SvLocal void
QBMWConfigHandlerParsePair(QBMWConfigHandler self, SvObject key, SvObject value, bool *changed, bool *notify)
{
    if (!SvObjectIsInstanceOf(key, SvString_getType())) {
        log_warning("%s() invalid type '%s'. SvString expected", __func__, SvObjectGetTypeName(key));
        return;
    }
    SvString keyStr = (SvString)key;
    log_debug("%s() key='%s'", __func__, SvStringCString(keyStr));

    QBMWConfigHandlerPassPairToPlugins(self, keyStr, value, changed, notify);

    if (SvStringEqualToCString(keyStr, "timezone")) { // update time zone
        if (!SvObjectIsInstanceOf(value, SvValue_getType()) || !SvValueIsString((SvValue) value)) {
            log_warning("%s: invalid answer (timezone), expected SvValueType_string", __func__);
            return;
        }

        const char *timeZone = SvValueGetStringAsCString((SvValue)value, NULL);
        if (timeZone && strcmp(timeZone, "") != 0) {
            log_state("Got time zone: '%s'", timeZone);
            QBConfigSet("TIMEZONE", timeZone);
            setenv("TZ", timeZone, 1);
            tzset();
        }
    } else if (SvStringEqualToCString(keyStr, "config")) {
        log_state("Got config");
        QBMWConfigHandlerHandleConfig(self, value);
    } else if (SvStringEqualToCString(keyStr, "vod_provider")) {
        if (!SvObjectIsInstanceOf(value, SvValue_getType()) || !SvValueIsString((SvValue) value)) {
            log_warning("%s: invalid answer (vod_provider). Expected SvValueString", __func__);
            return;
        }
        SvString newValue = NULL;
        if (SvStringEqualToCString(SvValueGetString((SvValue) value), "digitalsmiths")) {
            newValue = SvStringCreate("digitalsmiths", NULL);
        } else {
            newValue = SvStringCreate("cubimw", NULL);
        }
        const char* curVal = QBConfigGet("PROVIDERS.VOD");
        if (!SvStringEqualToCString(newValue, curVal)) {
            QBConfigSet("PROVIDERS.VOD", SvStringCString(newValue));
            QBConfigSave();
            SvValue message = SvValueCreateWithInteger(CONFIG_STATE_READ_CHANGE_NO_FIRSTBOOT, NULL);
            QBMWConfigHandlerNotify(self, message);
            SVRELEASE(message);
        }
        SVRELEASE(newValue);
    } else {
        log_debug("Notification from QBMWConfigMonitor ignored (key='%s')", SvStringCString(keyStr));
    }
}

SvLocal void
QBMWConfigHandlerGotResponse(SvGenericObject self_, SvHashTable results)
{
    QBMWConfigHandler self = (QBMWConfigHandler) self_;
    SvIterator it = SvHashTableKeysIterator(results);
    SvGenericObject key = NULL;
    bool hasVodProviderDefined = false;
    bool changed = false, notify = false;
    while ((key = SvIteratorGetNext(&it))) {
        SvGenericObject val = SvHashTableFind(results, key);
        if (SvObjectIsInstanceOf(key, SvString_getType()) && SvStringEqualToCString((SvString) key, "vod_provider")) {
            hasVodProviderDefined = true;
        }
        bool changed_ = false, notify_ = false;
        QBMWConfigHandlerParsePair(self, key, val, &changed_, &notify_);
        if (notify_)
            changed |= changed_;
        notify |= notify_;
    }
    if (!hasVodProviderDefined && QBVODLogicShouldResetProviderIfNotSpecifiedByMW()) {
        SvValue val = SvValueCreateWithString(SVSTRING("cubimw"), NULL);
        bool changed_ = false, notify_ = false;
        QBMWConfigHandlerParsePair(self, (SvObject) SVSTRING("vod_provider"), (SvObject) val, &changed_, &notify_);
        if (notify_)
            changed |= changed_;
        notify |= notify_;
        SVRELEASE(val);
    }
    if (notify) {
        SvValue message = SvValueCreateWithInteger(changed ? CONFIG_STATE_READ_CHANGE
                                                           : CONFIG_STATE_READ_NOCHANGE, NULL);
        QBMWConfigHandlerNotify(self, message);
        SVRELEASE(message);
    }
}

SvLocal void
QBMWConfigHandlerNoResponse(SvGenericObject self_)
{
    QBMWConfigHandler self = (QBMWConfigHandler) self_;
    SvValue message = SvValueCreateWithInteger(CONFIG_STATE_NOREAD, NULL);
    QBMWConfigHandlerNotify(self, message);
    SVRELEASE(message);
}

void
QBMWConfigHandlerAddPlugin(QBMWConfigHandler self, SvGenericObject newPlugin, SvString pluginName)
{
    if (!self || !newPlugin || !pluginName) {
        return;
    }

    SvHashTableInsert(self->plugins, (SvGenericObject) pluginName, newPlugin);
}

SvGenericObject
QBMWConfigHandlerGetPluginByName(QBMWConfigHandler self, SvString pluginName)
{
    return SvHashTableFind(self->plugins, (SvGenericObject) pluginName);
}

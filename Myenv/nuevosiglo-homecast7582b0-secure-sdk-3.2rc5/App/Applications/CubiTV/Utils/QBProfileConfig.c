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

#include "QBProfileConfig.h"

#include "QBConfig.h"
#include <QBConfigCore.h>
#include <libgen.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvType.h>

#include <lightjson/lightjson_types.h>
#include <lightjson/lightjson_object.h>
#include <lightjson/lightjson_array.h>
#include <lightjson/lightjson_parser.h>
#include <lightjson/lightjson_reader.h>
#include <lightjson/lightjson_writer.h>
#include <lightjson/lightjson_utils.h>

#include <limits.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define PROFILES_DIR    "/etc/vod/app/profiles/"

#define log_debug(fmt, ...)  if (0) SvLogNotice(COLBEG() "QBProfileConfig %s:%d " fmt COLEND_COL(green), __func__, __LINE__, ## __VA_ARGS__)

struct QBProfileConfig_ {
    struct SvObject_ super_;

    SvHashTable keyToListeners;
    SvWeakList globalListeners;

    bool dirty;
    char *fileName;
    struct lightjson_object *configuration;
};

static QBProfileConfig gProfileConfig = NULL;


SvLocal void
QBProfileConfigNotifyListeners(const char *key, const char *value)
{
    SvArray backup = SvWeakListCreateElementsList(gProfileConfig->globalListeners, NULL);
    SVAUTOSTRING(keyStr, key);
    SvWeakList queue = (SvWeakList) SvHashTableFind(gProfileConfig->keyToListeners, (SvObject) keyStr);
    if (queue) {
        if (SvWeakListIsEmpty(queue)) {
            SvHashTableRemove(gProfileConfig->keyToListeners, (SvObject) keyStr);
        } else {
            SvArray queueElements = SvWeakListCreateElementsList(queue, NULL);
            SvIterator it = SvArrayGetIterator(queueElements);
            SvArrayAddObjects(backup, &it);
            SVRELEASE(queueElements);
        }
    }

    SvIterator it = SvArrayGetIterator(backup);
    SvObject listener;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBConfigListener, listener, changed, key, value);
    }

    SVRELEASE(backup);
}

SvLocal void
QBProfileConfigNotifyAllListeners(void)
{
    SvIterator iter = SvHashTableGetKeysIterator(gProfileConfig->keyToListeners);
    SvString string;
    while ((string = (SvString) SvIteratorGetNext(&iter))) {
        const char *key = SvStringGetCString(string);
        QBProfileConfigNotifyListeners(key, QBProfileConfigGet(key));
    }
}

void
QBProfileConfigAddListener(SvObject listener, const char *key)
{
    if (key) {
        SvString str = SvStringCreate(key, NULL);
        SvWeakList queue = (SvWeakList) SvHashTableFind(gProfileConfig->keyToListeners, (SvObject) str);
        if (!queue) {
            queue = SvWeakListCreate(NULL);
            SvHashTableInsert(gProfileConfig->keyToListeners, (SvObject) str, (SvObject) queue);
            SVRELEASE(queue);
        }
        SvWeakListPushFront(queue, listener, NULL);
        SVRELEASE(str);
    } else
        SvWeakListPushFront(gProfileConfig->globalListeners, listener, NULL);
}

void
QBProfileConfigRemoveListener(SvObject listener, const char *key)
{
    if (!listener || !key)
        return;

    if (gProfileConfig->globalListeners)
        SvWeakListRemoveObject(gProfileConfig->globalListeners, listener);

    if (gProfileConfig->keyToListeners) {
        SVAUTOSTRING(str, key);
        SvWeakList queue = NULL;
        if ((queue = (SvWeakList) SvHashTableFind(gProfileConfig->keyToListeners, (SvObject) str)))
            SvWeakListRemoveObject(queue, listener);
    }
}

SvLocal int
QBProfileConfigSetProfile(const char *name)
{
    // save old config
    QBProfileConfigSave();

    if (gProfileConfig->configuration) {
        lightjson_object_destroy(gProfileConfig->configuration);
        gProfileConfig->configuration = NULL;
    }

    // open new config
    free(gProfileConfig->fileName);
    asprintf(&gProfileConfig->fileName, "%s%s", PROFILES_DIR, name);

    if (access(gProfileConfig->fileName, F_OK) == -1) {
        SvLogError("%s: Couldn't find profile configuration file: %s, making new empty configuration file.", __func__, gProfileConfig->fileName);
        struct lightjson_object *temp = lightjson_object_create();
        QBConfigCoreSaveConfiguration(temp, gProfileConfig->fileName, true);
        lightjson_object_destroy(temp);
    }

    if (!QBConfigCoreLoadConfiguration(gProfileConfig->fileName, &gProfileConfig->configuration)) {
        SvLogError("%s: Couldn't load profile configuration file: %s", __func__, gProfileConfig->fileName);
        gProfileConfig->configuration = NULL;
        free(gProfileConfig->fileName);
        gProfileConfig->fileName = NULL;
        return -1;
    }
    return 0;
}

SvLocal void
QBProfileConfigQBConfigChanged(SvObject self_, const char *key, const char *value)
{
    if (!strcmp(key, "UI.lastUserProfile")) {
        QBProfileConfigSetProfile(value);
        QBProfileConfigNotifyAllListeners();
    }
}

int
QBProfileConfigSave(void)
{
    int re = QBConfigSave();
    if (re < 0)
        return re;

    if (gProfileConfig->dirty && gProfileConfig->configuration && gProfileConfig->fileName) {
        QBConfigCoreSaveConfiguration(gProfileConfig->configuration, gProfileConfig->fileName, true);
        gProfileConfig->dirty = false;
    }

    return 0;
}

QBProfileConfigSetResult
QBProfileConfigSet(const char *key, const char *value)
{
    // we call QBConfigSet() always so value is preserved, this way QBProfileConfig will work even if user profile is not set
    QBConfigSetResult re = QBConfigSet(key, value);

    if (!gProfileConfig->configuration) {
        switch (re) {
            case QBConfigSetResult_valueForKeyIsAlreadySet:
                return QBProfileConfigSetResult_valueForKeyIsAlreadySet;
            case QBConfigSetResult_fail:
            default:
                return QBProfileConfigSetResult_fail;
            case QBConfigSetResult_succes:
                QBProfileConfigNotifyListeners(key, value);
                return QBProfileConfigSetResult_succes;
        }
    }

    log_debug("Setting a key: (%s) with value: (%s)", key, (value) ? value : "null");

    if ((re == QBConfigSetResult_succes || re == QBConfigSetResult_valueForKeyIsAlreadySet)) {
        // we check if key is already there
        lightjson_value oldvalue;
        lightjson_type type = QBConfigCoreFindKeyValueInConfiguration(gProfileConfig->configuration, key, &oldvalue);
        if (type == lightjson_type_invalid && !value) {
            return QBProfileConfigSetResult_fail;
        } else {
            const char *oldValue = (type == lightjson_type_string) ? oldvalue.string_value.str : NULL;

            if (oldValue && value && !strcmp(oldValue, value)) {
                // value was not changed
                log_debug("value was not changed");
                return QBProfileConfigSetResult_valueForKeyIsAlreadySet;
            }
            log_debug("value is updated, save new value, old(%s), new(%s)", oldValue, value);
        }

        gProfileConfig->dirty = true;
        QBConfigCoreSetKeyIntoConfiguration(gProfileConfig->configuration, key, value);
        QBProfileConfigNotifyListeners(key, value);
        return QBProfileConfigSetResult_succes;
    }

    return QBProfileConfigSetResult_fail;
}

const char *
QBProfileConfigGet(const char *key)
{
    log_debug("Getting a key: %s", key);

    if (gProfileConfig->configuration) {
        lightjson_value value;
        lightjson_type val_type = QBConfigCoreFindKeyValueInConfiguration(gProfileConfig->configuration, key, &value);
        if (val_type != lightjson_type_invalid) {
            return (val_type == lightjson_type_string) ? (value.string_value.str) : NULL;
        }
    }

    return QBConfigGet(key);
}

int
QBProfileConfigGetInteger(const char *key, int * value)
{
    const char* strVal = QBProfileConfigGet(key);
    if (!strVal) {
        return -1;
    }

    errno = 0;
    char * convertionEnd;
    const long long tmp = strtoll(strVal, &convertionEnd, 0);
    if (strVal == convertionEnd || errno != 0) {
        // not a decimal number
        return -1;
    } else if (*convertionEnd != '\0') {
        // extra characters at end of input
        return -1;
    } else if ((tmp > INT_MAX) || (tmp < INT_MIN)) {
        // out of range of type int
        return -1;
    }

    *value = tmp;
    return 0;
}

SvLocal SvType QBProfileConfig_getType(void);

void
QBProfileConfigInit(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    gProfileConfig = (QBProfileConfig) SvTypeAllocateInstance(QBProfileConfig_getType(), &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "Can't allocate QBProfileConfig");
        goto err;
    }

    mkdir(PROFILES_DIR, 0777);

    gProfileConfig->globalListeners = SvWeakListCreate(NULL);
    gProfileConfig->keyToListeners = SvHashTableCreate(100, NULL);

    const char* profileName = QBConfigGet("UI.lastUserProfile");
    if (profileName)
        QBProfileConfigSetProfile(profileName);

    QBConfigAddListener((SvObject) gProfileConfig, NULL);
err:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBProfileConfigDestroyInternal(void *self_)
{
    QBProfileConfig self = self_;
    SVRELEASE(self->globalListeners);
    SVRELEASE(self->keyToListeners);
    free(self->fileName);
    if (self->configuration) {
        lightjson_object_destroy(self->configuration);
        self->configuration = NULL;
    }
}

void
QBProfileConfigDestroy(void)
{
    SVTESTRELEASE(gProfileConfig);
}

SvLocal SvType
QBProfileConfig_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBProfileConfigDestroyInternal
    };

    static const struct QBConfigListener_t configMethods = {
        .changed = QBProfileConfigQBConfigChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBProfileConfig",
                            sizeof(struct QBProfileConfig_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBConfigListener_getInterface(), &configMethods,
                            NULL);
    }

    return type;
}

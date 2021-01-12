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

#include "QBMWConfigHandlerPluginAppMode.h"

#include <QBConf.h>
#include <Services/QBMWConfigHandlerPlugins/QBMWConfigHandlerPlugin.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>

SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 0, "QBMWConfigHandlerPluginAppModeLogLevel", "" );

#define log_error(fmt, ...) if (env_log_level() >= 0) { SvLogError( COLBEG() "QBMWConfigHandlerPluginAppMode:: " fmt COLEND_COL(red), ##__VA_ARGS__); };
#define log_warning(fmt, ...) if (env_log_level() >= 0) { SvLogWarning( COLBEG() "QBMWConfigHandlerPluginAppMode:: " fmt COLEND_COL(yellow), ##__VA_ARGS__); };
#define log_state(fmt, ...) if (env_log_level() >= 1) { SvLogNotice( COLBEG() "QBMWConfigHandlerPluginAppMode :: " fmt COLEND_COL(blue), ##__VA_ARGS__); };
#define log_debug(fmt, ...) if (env_log_level() >= 2) { SvLogNotice( COLBEG() "QBMWConfigHandlerPluginAppMode :: " fmt COLEND_COL(green), ##__VA_ARGS__); };

struct QBMWConfigHandlerPluginAppMode_ {
    struct SvObject_ super_;
    SvString mode;
    bool isFirstTimeRequired;
};

SvLocal void
QBMWConfigHandlerPluginAppModeHandleConfigPair(SvObject self_, SvString key, SvObject value, bool *changed_, bool *notify_)
{
    QBMWConfigHandlerPluginAppMode self = (QBMWConfigHandlerPluginAppMode) self_;
    bool changed = false, notify = false;

    if (!SvStringEqualToCString(key, "mode"))
        goto fini;

    if (!SvObjectIsInstanceOf(value, SvValue_getType()) || !SvValueIsString((SvValue) value)) {
        log_warning("%s: invalid answer (mode), expected SvValueType_string", __func__);
        goto fini;
    }

    SvString mode = (SvString) SvValueGetString((SvValue) value);
    if ((!SvStringEqualToCString(mode, "IP")) &&
        (!SvStringEqualToCString(mode, "HYBRID"))) {
        log_warning("%s: invalid stb_mode: %s", __func__, SvStringCString(mode));
        goto fini;
    }

    log_state("Got STB mode: '%s'", SvStringCString(mode));

    const char *currentMode = QBConfigGet("HYBRIDMODE");
    bool isModeChanged = !SvStringEqualToCString(mode, currentMode);

    log_debug("isModeChanged: %d, isFirstTimeRequired: %d", isModeChanged, self->isFirstTimeRequired);
    if (isModeChanged || !self->isFirstTimeRequired) {
        self->isFirstTimeRequired = true;

        QBConfigSet("HYBRIDMODE", SvStringCString(mode));
        QBConfigSave();

        notify = true;
        changed = isModeChanged;
    }

fini:
    if (changed_)
        *changed_ = changed;
    if (notify_)
        *notify_ = notify;
}

SvLocal SvType
QBMWConfigHandlerPluginAppMode_getType(void)
{
    static SvType type = NULL;

    static const struct QBMWConfigHandlerPlugin_ mwConfigHandlerMethods = {
        .handleConfigPair = QBMWConfigHandlerPluginAppModeHandleConfigPair
    };

    if (!type) {
        SvTypeCreateManaged("QBMWConfigHandlerPluginAppMode",
                            sizeof(struct QBMWConfigHandlerPluginAppMode_),
                            SvObject_getType(), &type,
                            QBMWConfigHandlerPlugin_getInterface(), &mwConfigHandlerMethods,
                            NULL);
    }

    return type;
}

QBMWConfigHandlerPluginAppMode
QBMWConfigHandlerPluginAppModeCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBMWConfigHandlerPluginAppMode self =
            (QBMWConfigHandlerPluginAppMode) SvTypeAllocateInstance(QBMWConfigHandlerPluginAppMode_getType(),
                                                                         &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate " QBMW_CONFIG_HANDLER_PLUGIN_APPMODE);
        goto fini;
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

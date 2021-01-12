/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2018 Cubiware Sp. z o.o. All rights reserved.
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

#include "MagineQBMWConfigHandlerPlugin.h"

#include <Services/QBMWConfigHandlerPlugins/QBMWConfigHandlerPlugin.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <stdbool.h>

SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 0, "QBMWConfigLogLevel", "" );

#define log_error(fmt, ...) if (env_log_level() >= 0) { SvLogError( COLBEG() "QBInnov8onConfigHandler:: " fmt COLEND_COL(red), ##__VA_ARGS__); };
#define log_warning(fmt, ...) if (env_log_level() >= 0) { SvLogWarning( COLBEG() "QBInnov8onConfigHandler:: " fmt COLEND_COL(yellow), ##__VA_ARGS__); };
#define log_state(fmt, ...) if (env_log_level() >= 1) { SvLogNotice( COLBEG() "QBInnov8onConfigHandler :: " fmt COLEND_COL(blue), ##__VA_ARGS__); };
#define log_debug(fmt, ...) if (env_log_level() >= 2) { SvLogNotice( COLBEG() "QBInnov8onConfigHandler :: " fmt COLEND_COL(green), ##__VA_ARGS__); };

struct MagineQBMWConfigHandlerPlugin_ {
    struct SvObject_ super_;

    SvString magineApiToken;
    SvString magineSecret;
    SvString magineDeviceId;
};

SvLocal void
MagineQBMWConfigHandlerPluginHandleConfigPair(SvObject self_, SvString key, SvObject value, bool *changed, bool *notify)
{
    MagineQBMWConfigHandlerPlugin self = (MagineQBMWConfigHandlerPlugin) self_;

    if (!SvStringEqualToCString(key, "magineAPIToken") &&
        !SvStringEqualToCString(key, "magineSecret") &&
        !SvStringEqualToCString(key, "magineDeviceID")) {
        goto fini;
    }

    if (!SvObjectIsInstanceOf(value, SvValue_getType())) {
        log_warning("%s():%d invalid answer (%s is expected to be SvValue)", __func__, __LINE__, SvStringCString(key));
        goto fini;
    }

    if (!SvValueIsString((SvValue) value)) {
        log_warning("%s():%d invalid answer (%s is expected to be SvValueType_string)", __func__, __LINE__, SvStringCString(key));
        goto fini;
    }

    SvString valueString = SvValueGetString((SvValue) value);

    if (SvStringEqualToCString(key, "magineAPIToken")) {
        SVTESTRELEASE(self->magineApiToken);
        self->magineApiToken = SVRETAIN(valueString);
    } else if (SvStringEqualToCString(key, "magineSecret")) {
        SVTESTRELEASE(self->magineApiToken);
        self->magineSecret = SVRETAIN(valueString);
    } else if (SvStringEqualToCString(key, "magineDeviceID")) {
        SVTESTRELEASE(self->magineApiToken);
        self->magineDeviceId = SVRETAIN(valueString);
    }

    log_state("%s in %s", SvStringCString(key), SvStringCString(valueString));

fini:
    if (notify) {
        *notify = false;
    }
    if (changed) {
        *changed = false;
    }
}

SvLocal void
MagineQBMWConfigHandlerPluginDestroy(void *self_)
{
    MagineQBMWConfigHandlerPlugin self = self_;

    SVTESTRELEASE(self->magineApiToken);
    SVTESTRELEASE(self->magineDeviceId);
    SVTESTRELEASE(self->magineSecret);
}

SvLocal SvType
MagineQBMWConfigHandlerPlugin_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = MagineQBMWConfigHandlerPluginDestroy
    };

    static const struct QBMWConfigHandlerPlugin_ mwConfigHandlerMethods = {
        .handleConfigPair = MagineQBMWConfigHandlerPluginHandleConfigPair
    };

    if (!type) {
        SvTypeCreateManaged("MagineQBMWConfigHandlerPlugin",
                            sizeof(struct MagineQBMWConfigHandlerPlugin_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBMWConfigHandlerPlugin_getInterface(), &mwConfigHandlerMethods,
                            NULL);
    }

    return type;
}

MagineQBMWConfigHandlerPlugin
MagineQBMWConfigHandlerPluginCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    MagineQBMWConfigHandlerPlugin self =
        (MagineQBMWConfigHandlerPlugin) SvTypeAllocateInstance(MagineQBMWConfigHandlerPlugin_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate MagineQBMWConfigHandlerPlugin");
        goto fini;
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

SvString
MagineQBMWConfigHandlerPluginGetMagineApiToken(MagineQBMWConfigHandlerPlugin self)
{
    return self->magineApiToken;
}

SvString
MagineQBMWConfigHandlerPluginGetMagineSecret(MagineQBMWConfigHandlerPlugin self)
{
    return self->magineSecret;
}

SvString
MagineQBMWConfigHandlerPluginGetMagineDeviceId(MagineQBMWConfigHandlerPlugin self)
{
    return self->magineDeviceId;
}

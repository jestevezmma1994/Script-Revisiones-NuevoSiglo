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

#include "ConaxQBMWConfigHandlerPlugin.h"

#include <Services/QBMWConfigHandlerPlugins/QBMWConfigHandlerPlugin.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <QBConf.h>

SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 0, "QBMWConfigLogLevel", "" );

#define log_error(fmt, ...) if (env_log_level() >= 0) { SvLogError( COLBEG() "QBInnov8onConfigHandler:: " fmt COLEND_COL(red), ##__VA_ARGS__); };
#define log_warning(fmt, ...) if (env_log_level() >= 0) { SvLogWarning( COLBEG() "QBInnov8onConfigHandler:: " fmt COLEND_COL(yellow), ##__VA_ARGS__); };
#define log_state(fmt, ...) if (env_log_level() >= 1) { SvLogNotice( COLBEG() "QBInnov8onConfigHandler :: " fmt COLEND_COL(blue), ##__VA_ARGS__); };
#define log_debug(fmt, ...) if (env_log_level() >= 2) { SvLogNotice( COLBEG() "QBInnov8onConfigHandler :: " fmt COLEND_COL(green), ##__VA_ARGS__); };

struct ConaxQBMWConfigHandlerPlugin_ {
    struct SvObject_ super_;
};

SvLocal void
ConaxQBMWConfigHandlerPluginHandleConfigPair(SvObject self_, SvString key, SvObject value, bool *changed, bool *notify)
{
    // PlayReady Operator ID
    if (!SvStringEqualToCString(key, "operator_id")) {
        goto fini;
    }

    if (!SvObjectIsInstanceOf(value, SvValue_getType()) || !SvValueIsString((SvValue) value)) {
        log_warning("%s: invalid answer (operatorID), expected SvValueType_string. PlayReady will not work.", __func__);
        goto fini;
    }

    SvString playReadyOperatorId = SvValueGetString((SvValue)value);
    const char *oldOperatorId = QBConfigGet("CX_PR_OPERATOR_ID");
    if (oldOperatorId && !SvStringEqualToCString(playReadyOperatorId, oldOperatorId)) {
        QBConfigSet("CX_PR_OPERATOR_ID", SvStringCString(playReadyOperatorId));
        QBConfigSave();
    }

    log_state("Got PlayReady operator id: '%s'", SvStringCString(playReadyOperatorId));

fini:
    if (notify)
        *notify = false;
    if (changed)
        *changed = false;
}

SvLocal SvType
ConaxQBMWConfigHandlerPlugin_getType(void)
{
    static SvType type = NULL;

    static const struct QBMWConfigHandlerPlugin_ mwConfigHandlerMethods = {
        .handleConfigPair = ConaxQBMWConfigHandlerPluginHandleConfigPair
    };

    if (!type) {
        SvTypeCreateManaged("ConaxQBMWConfigHandlerPlugin",
                            sizeof(struct ConaxQBMWConfigHandlerPlugin_),
                            SvObject_getType(), &type,
                            QBMWConfigHandlerPlugin_getInterface(), &mwConfigHandlerMethods,
                            NULL);
    }

    return type;
}

ConaxQBMWConfigHandlerPlugin
ConaxQBMWConfigHandlerPluginCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    ConaxQBMWConfigHandlerPlugin self = (ConaxQBMWConfigHandlerPlugin)SvTypeAllocateInstance(ConaxQBMWConfigHandlerPlugin_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate ConaxQBMWConfigHandlerPlugin");
        goto fini;
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

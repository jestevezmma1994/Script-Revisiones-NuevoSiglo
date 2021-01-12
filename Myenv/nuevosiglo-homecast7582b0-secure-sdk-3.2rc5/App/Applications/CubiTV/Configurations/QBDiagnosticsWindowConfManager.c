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

#include "QBDiagnosticsWindowConfManager.h"
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvLog.h>
#include <settings.h>
#include <main.h>
#include <safeFiles.h> // QBFileToBuffer
#include <SvJSON/SvJSONParse.h> // SvJSONParseString
#include <SvJSON/SvJSONClassRegistry.h>
#include <QBConfig.h>

SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 0, "DiagnosticsLogLevel", "" );

#define log_error(fmt, ...) do { if (env_log_level() >= 0) SvLogError(  COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); } while (0)
#define log_state(fmt, ...) do { if (env_log_level() >= 1) SvLogNotice( COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ##__VA_ARGS__); } while (0)
#define log_debug(fmt, ...) do { if (env_log_level() >= 2) SvLogNotice(          "[%s] " fmt, __func__, ##__VA_ARGS__); } while (0)


/*
 * QBDiagnosticInfo helper class
 */

struct QBDiagnosticInfo_t {
    struct SvObject_ super_;
    SvString id;
    SvString label;
};

SvLocal void
QBDiagnosticInfo__dtor__(void *self_)
{
    QBDiagnosticInfo self = self_;
    SVTESTRELEASE(self->id);
    SVTESTRELEASE(self->label);
}

SvLocal SvType
QBDiagnosticInfo_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiagnosticInfo__dtor__
    };
    static SvType type = NULL;
    if ( !type ) {
        SvTypeCreateManaged("QBDiagnosticInfo",
                            sizeof(struct QBDiagnosticInfo_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvString
QBDiagnosticInfoGetId(QBDiagnosticInfo self)
{
    return self->id;
}

SvString
QBDiagnosticInfoGetLabel(QBDiagnosticInfo self)
{
    return self->label;
}

/*
 * QBDiagnosticsWindowConf class
 */

struct QBDiagnosticsWindowConf_t {
    struct SvObject_ super_;
    SvString id;
    SvString caption;

    SvHashTable diagnosticsLookup;
    SvArray enabledDiagnostics;
};

SvLocal void
QBDiagnosticsWindowConf__dtor__(void *self_)
{
    QBDiagnosticsWindowConf self = self_;
    SVTESTRELEASE(self->id);
    SVTESTRELEASE(self->caption);
    SVTESTRELEASE(self->enabledDiagnostics);
    SVTESTRELEASE(self->diagnosticsLookup);
}

SvLocal SvType
QBDiagnosticsWindowConf_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiagnosticsWindowConf__dtor__
    };
    static SvType type = NULL;
    if ( !type ) {
        SvTypeCreateManaged("QBDiagnosticsWindowConf",
                            sizeof(struct QBDiagnosticsWindowConf_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal bool
checkBool(SvValue value)
{
    if (!value || !SvObjectIsInstanceOf((SvObject) value, SvValue_getType()) || !SvValueIsBoolean(value)) {
        return false;
    }
    return true;
}

SvLocal bool
checkString(SvValue value)
{
    if (!value || !SvObjectIsInstanceOf((SvObject) value, SvValue_getType()) || !SvValueIsString(value)) {
        return false;
    }
    return true;
}

SvLocal SvGenericObject
QBDiagnosticsWindowConfLoad(SvGenericObject helper_, SvString className, SvHashTable desc,
    SvErrorInfo *errorOut)
{
    SvValue value = NULL;
    QBDiagnosticsWindowConf conf = (QBDiagnosticsWindowConf) SvTypeAllocateInstance(QBDiagnosticsWindowConf_getType(), NULL);

    SvValue tunerType = NULL;
    const char* tunerTypeStr = QBConfigGet("DEFAULTTUNERTYPE");
    if (tunerTypeStr) {
        char *type = strdup(tunerTypeStr);
        if (strlen(type) > 3)
            type[3] = 0;
        tunerType = SvValueCreateWithCString(type, NULL);
        free(type);
    }

    log_debug("Parsing %s section", className ? SvStringCString(className) : "");

    if ( !checkString((value = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("caption")))) ) {
        log_error("Failed(caption)");
        goto fini;
    }
    conf->caption = SVRETAIN(SvValueGetString(value));

    if ( !checkString((value = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("id")))) ) {
        log_error("Failed(id)");
        goto fini;
    }
    conf->id = SVRETAIN(SvValueGetString(value));

    conf->diagnosticsLookup = SvHashTableCreate(17, NULL);
    conf->enabledDiagnostics = SvArrayCreate(NULL);
    SvArray diagnostics = (SvArray) SvHashTableFind(desc, (SvObject) SVSTRING("diagnostics"));
    if (!diagnostics || !SvObjectIsInstanceOf((SvObject) diagnostics, SvArray_getType())) {
        log_error("Failed(diagnostics)");
        goto fini;
    }

    SvIterator it = SvArrayIterator(diagnostics);
    SvHashTable diag = NULL;
    while ((diag = (SvHashTable)SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf((SvObject) diag, SvHashTable_getType())) {
            continue;
        }
        SvValue enabledV = (SvValue) SvHashTableFind(diag, (SvGenericObject)SVSTRING("enabled"));
        if ( !checkBool(enabledV) || !SvValueGetBoolean(enabledV) ) {
            continue;
        }

        SvArray hideArr = (SvArray) SvHashTableFind(diag, (SvGenericObject)SVSTRING("hideForTunerType"));
        if (hideArr && SvObjectIsInstanceOf((SvObject) hideArr, SvArray_getType())) {
            if (SvArrayContainsObject(hideArr, (SvObject) tunerType))
                continue;
        }

        SvValue idV = (SvValue) SvHashTableFind(diag, (SvGenericObject)SVSTRING("id"));
        SvValue labelV = (SvValue) SvHashTableFind(diag, (SvGenericObject)SVSTRING("label"));
        if (!checkString(idV)) { // label is not mandatory
            continue;
        }

        QBDiagnosticInfo diagnosticInfo = (QBDiagnosticInfo) SvTypeAllocateInstance(QBDiagnosticInfo_getType(), NULL);
        diagnosticInfo->id = SVRETAIN(SvValueGetString(idV));
        if (labelV) {
            diagnosticInfo->label = SVRETAIN(SvValueGetString(labelV));
        }
        SvArrayAddObject(conf->enabledDiagnostics, (SvGenericObject)diagnosticInfo);
        SvHashTableInsert(conf->diagnosticsLookup, (SvGenericObject)diagnosticInfo->id, (SvGenericObject)diagnosticInfo);
        SVRELEASE(diagnosticInfo);
    }
    SVTESTRELEASE(tunerType);
    return (SvGenericObject) conf;

fini:
    SVRELEASE(conf);
    return NULL;
}

/**
 * QBDiagnosticsManager class
 */

struct QBDiagnosticsManager_t {
    struct SvObject_ super_;
    SvHashTable configurations;
    AppGlobals appGlobals;
};

SvLocal void
QBDiagnosticsManager__dtor__(void *self_)
{
    QBDiagnosticsManager self = self_;
    SVRELEASE(self->configurations);
    self->configurations = NULL;
}

SvLocal SvType
QBDiagnosticsManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiagnosticsManager__dtor__
    };
    static SvType type = NULL;
    if ( !type ) {
        SvTypeCreateManaged("QBDiagnosticsManager",
                            sizeof(struct QBDiagnosticsManager_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void
QBDiagnosticsManagerLoadConfiguration(QBDiagnosticsManager self)
{
    char *buffer = NULL;
    SvArray confs = NULL;

    SvString fileName = QBInitLogicGetDiagnosticsOptionsFileName(self->appGlobals->initLogic);
    if ( !fileName ) {
        return;
    }

    QBFileToBuffer(SvStringCString(fileName), &buffer);
    SVRELEASE(fileName);
    if ( !buffer ) {
        goto fin;
    }

    if ( !(confs = (SvArray) SvJSONParseString(buffer, true, NULL)) ) {
        goto fin;
    }

    SvHashTableRemoveAllObjects(self->configurations);

    SvIterator it = SvArrayIterator(confs);
    QBDiagnosticsWindowConf conf;
    while ( (conf = (QBDiagnosticsWindowConf) SvIteratorGetNext(&it)) ) {
        SvHashTableInsert(self->configurations, (SvGenericObject) conf->id, (SvGenericObject) conf);
    }

fin:
    SVTESTRELEASE(confs);
    free(buffer);
}

QBDiagnosticsWindowConf
QBDiagnosticsManagerGetConf(QBDiagnosticsManager self, SvString id)
{
    if ( !self || !self->configurations || !id ) {
        log_error("sanity check failed");
        return NULL;
    }
    return (QBDiagnosticsWindowConf) SvHashTableFind(self->configurations, (SvGenericObject) id);
}

SvArray
QBDiagnosticsWindowConfGetDiagnostics(QBDiagnosticsWindowConf self)
{
    return self->enabledDiagnostics;
}

bool
QBDiagnosticsWindowConfIsEnabled(QBDiagnosticsWindowConf self,  SvString diagnosticId)
{
    return SvHashTableFind(self->diagnosticsLookup, (SvGenericObject)diagnosticId);
}

SvString
QBDiagnosticsWindowConfGetCaption(QBDiagnosticsWindowConf self)
{
    return self->caption;
}

QBDiagnosticsManager
QBDiagnosticsManagerCreate(AppGlobals appGlobals)
{
    QBDiagnosticsManager self = (QBDiagnosticsManager)
        SvTypeAllocateInstance(QBDiagnosticsManager_getType(), NULL);

    self->appGlobals = appGlobals;
    self->configurations = SvHashTableCreate(11, NULL);

    SvJSONClass helper = SvJSONClassCreate(NULL, QBDiagnosticsWindowConfLoad, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBDiagnosticsWindowConf"), (SvGenericObject) helper, NULL);
    SVRELEASE(helper);

    QBDiagnosticsManagerLoadConfiguration(self);

    return self;
}


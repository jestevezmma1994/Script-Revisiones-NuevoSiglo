/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#include "InputTranslator.h"

#include <main.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>

#include <SvJSON/SvJSONParse.h>

#include <QBInput/QBInputService.h>
#include <QBInput/QBInputDevice.h>
#include <QBInput/QBInputUtils.h>
#include <QBInput/Filters/QBMultiPressGenerator.h>

#include <QBApplicationController.h>

#include <safeFiles.h>

struct QBKeyInputTranslator_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    bool started;

    QBMultiPressGenerator inputGenerator;
    SvArray rules;
};


SvLocal void QBKeyInputTranslatorSwitch(QBKeyInputTranslator self, QBWindowContext ctx, bool on)
{
    const char *oldType = SvObjectGetTypeName((SvGenericObject) ctx);
    size_t count = SvArrayCount(self->rules);
    SvHashTable rule;
    SvArray contexts;
    SvString typeName = SvStringCreate(oldType, NULL);
    SvValue type = SvValueCreateWithString(typeName, NULL);

    for (size_t i = 0; i < count; i++) {
        rule = (SvHashTable) SvArrayAt(self->rules, i);
        contexts = (SvArray) SvHashTableFind(rule, (SvGenericObject) SVSTRING("contexts"));
        if (contexts && SvObjectIsInstanceOf((SvObject) contexts, SvArray_getType())) {
            if (SvArrayContainsObject(contexts, (SvGenericObject) type))
                QBMultiPressGeneratorEnableMapping(self->inputGenerator, i, true);
        }
    }

    SVRELEASE(type);
    SVRELEASE(typeName);
}


SvLocal void QBKeyInputTranslatorSwitchStarted(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    QBKeyInputTranslator self = (QBKeyInputTranslator) self_;

    //if (from)
    //    QBKeyInputTranslatorSwitch(self, from, false);

    size_t count = SvArrayCount(self->rules);
    for (size_t i = 0; i < count; i++)
        QBMultiPressGeneratorEnableMapping(self->inputGenerator, i, false);


}

SvLocal void QBKeyInputTranslatorSwitchEnded(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    QBKeyInputTranslator self = (QBKeyInputTranslator) self_;

    if (to)
        QBKeyInputTranslatorSwitch(self, to, true);
}


SvLocal SvType
QBKeyInputTranslator_getType(void)
{
    static SvType type = NULL;
    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBKeyInputTranslatorSwitchStarted,
        .ended = QBKeyInputTranslatorSwitchEnded
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBKeyInputTranslator", sizeof(struct QBKeyInputTranslator_t),
                            SvObject_getType(), &type,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            NULL);
    }

    return type;
}

QBKeyInputTranslator
QBKeyInputTranslatorCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    QBKeyInputTranslator self = NULL;
    SvErrorInfo error = NULL;

    if (!(self = (QBKeyInputTranslator) SvTypeAllocateInstance(QBKeyInputTranslator_getType(), &error)))
        goto out;
    self->appGlobals = appGlobals;

out:
    SvErrorInfoPropagate(error, errorOut);
    if (error) {
        SVTESTRELEASE(self);
        return NULL;
    }
    return self;
}



SvLocal QBMultiPressMapping *generate_map(SvArray rules)
{
    QBMultiPressMapping *map = NULL;
    SvHashTable rule;
    SvValue src, dest, srcInt, destInt;
    size_t count;
    uint32_t *outTab;
    SvHashTable names2codes, codes2names;

    if (!rules || !(count = SvArrayCount(rules)))
        return NULL;

    if (!(names2codes = SvHashTableCreate(617, NULL)))
        return NULL;

    codes2names = QBInputLoadCodesMap(names2codes);
    SVTESTRELEASE(codes2names);

    if (!(map = (QBMultiPressMapping *) malloc((count + 1) * sizeof(QBMultiPressMapping) + 2 * count * sizeof(uint32_t))))
        goto err;
    outTab = (uint32_t *) (void *) ((char *)map + (count + 1) * sizeof(QBMultiPressMapping));

    for (size_t i = 0; i < count; i++) {
        rule = (SvHashTable) SvArrayAt(rules, i);
        if (!SvObjectIsInstanceOf((SvObject) rule, SvHashTable_getType()))
            goto err;

        src = (SvValue) SvHashTableFind(rule, (SvObject) SVSTRING("input"));
        dest = (SvValue) SvHashTableFind(rule, (SvObject) SVSTRING("output"));

        if (!src || !dest || !SvObjectIsInstanceOf((SvObject) src, SvValue_getType()) || !SvObjectIsInstanceOf((SvObject) dest, SvValue_getType()))
            goto err;

        srcInt = (SvValue) SvHashTableFind(names2codes, (SvObject) SvValueGetString(src));
        destInt = (SvValue) SvHashTableFind(names2codes, (SvObject) SvValueGetString(dest));

        if (!srcInt || !destInt)
            goto err;

        map[i].srcCode = SvValueGetInteger(srcInt);
        map[i].enabled = false;
        map[i].outputSequence = outTab;
        outTab[0] = SvValueGetInteger(destInt);
        outTab[1] = 0;

        outTab += 2;
    }
    map[count].srcCode = 0;
    map[count].enabled = false;
    map[count].outputSequence = NULL;

    SVTESTRELEASE(names2codes);
    return map;

err:
    SVTESTRELEASE(names2codes);
    free(map);
    return NULL;
}






int
QBKeyInputTranslatorStart(QBKeyInputTranslator self, SvString confFile, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    char *buffer = NULL;
    SvArray rules = NULL;
    QBMultiPressMapping *map = NULL;

    if (!self || !confFile) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Self or conf file is NULL!");
        goto out;
    }

    if (self->started)
        return 0;

    QBFileToBuffer(SvStringCString(confFile), &buffer);
    if (!buffer) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "conf file %s couldn't be loaded!", SvStringCString(confFile));
        goto out;
    }

    rules = (SvArray) SvJSONParseString(buffer, true, &error);

    free(buffer);
    buffer = NULL;

    if (error || !rules || !SvObjectIsInstanceOf((SvObject) rules, SvArray_getType())) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "couldn't parse rules");
        goto out;
    }

    SVTESTRELEASE(self->rules);
    self->rules = rules;


    SVTESTRELEASE(self->inputGenerator);
    if (!(self->inputGenerator = (QBMultiPressGenerator) SvTypeAllocateInstance(QBMultiPressGenerator_getType(), &error))) {
        goto out;
    }

    if (!(map = generate_map(rules))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "couldn't generate map");
        goto out;
    }

    if (!(QBMultiPressGeneratorInit(self->inputGenerator, map, &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "couldn't initialize input filter");
        goto out;
    }

    QBInputServiceAddGlobalFilter((SvGenericObject) self->inputGenerator, NULL, NULL);

    QBApplicationControllerAddListener(self->appGlobals->controller, (SvGenericObject) self);

    self->started = true;

out:
    if (map)
        free(map);
    SvErrorInfoPropagate(error, errorOut);
    if (error)
        return -1;
    return 0;
}

int
QBKeyInputTranslatorStop(QBKeyInputTranslator self, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto out;
    }

    if (!self->started)
        return 0;

    QBApplicationControllerRemoveListener(self->appGlobals->controller, (SvGenericObject) self);

    QBInputServiceRemoveFilter((SvGenericObject) self->inputGenerator, NULL);

    SVTESTRELEASE(self->rules);
    SVTESTRELEASE(self->inputGenerator);

    self->rules = NULL;
    self->inputGenerator = NULL;

    self->started = false;

out:
    SvErrorInfoPropagate(error, errorOut);
    if (error) {
        return -1;
    }
    return 0;
}


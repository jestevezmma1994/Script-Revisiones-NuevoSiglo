/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "QBAudioStreamDesc.h"
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <dataformat/sv_data_format.h>
#include <SvJSON/SvJSONClassRegistry.h>


SvType
QBAudioStreamDesc_getType(void)
{
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBAudioStreamDesc", sizeof(struct QBAudioStreamDesc_),
                            SvObject_getType(), &type, NULL);
    }
    return type;
}

SvLocal void
saveQBAudioStreamDesc(SvGenericObject helper_, SvGenericObject userParams, SvHashTable desc, SvErrorInfo *errorOut)
{
    QBAudioStreamDesc self = (QBAudioStreamDesc) userParams;

    SvValue tmp = SvValueCreateWithInteger(self->codec, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("codec"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    tmp = SvValueCreateWithBoolean(self->isMultiChannel, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("multiChannel"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    SvString lang = SvStringCreate(self->lang, NULL);
    tmp = SvValueCreateWithString(lang, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("lang"), (SvGenericObject) tmp);
    SVRELEASE(tmp);
    SVRELEASE(lang);

    tmp = SvValueCreateWithInteger(self->type, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("type"), (SvGenericObject) tmp);
    SVRELEASE(tmp);
}

SvLocal SvObject
loadQBAudioStreamDesc(SvObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    SvValue codec = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("codec"));
    SvValue isMultiChannel = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("multiChannel"));
    SvValue lang = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("lang"));
    SvValue type = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("type"));

    if (!codec || !isMultiChannel || !lang || !type)
        return NULL;

    if (!SvObjectIsInstanceOf((SvObject) codec, SvValue_getType()) ||
        !SvObjectIsInstanceOf((SvObject) isMultiChannel, SvValue_getType()) ||
        !SvObjectIsInstanceOf((SvObject) lang, SvValue_getType()) ||
        !SvObjectIsInstanceOf((SvObject) type, SvValue_getType()))
        return NULL;

    if (SvValueGetType(codec) != SvValueType_integer ||
        SvValueGetType(isMultiChannel) != SvValueType_boolean ||
        SvValueGetType(lang) != SvValueType_string ||
        SvValueGetType(type) != SvValueType_integer)
        return NULL;

    struct sv_simple_audio_format audio;
    audio.codec = SvValueGetInteger(codec);
    audio.is_multichannel = SvValueGetBoolean(isMultiChannel);
    audio.type = SvValueGetInteger(type);

    audio.lang = (char *) SvValueGetStringAsCString(lang, NULL);

    QBAudioStreamDesc audioDesc = QBAudioStreamDescCreate(0, &audio, NULL);

    return (SvObject) audioDesc;
}

QBAudioStreamDesc
QBAudioStreamDescCreate(unsigned int trackIndex,
                        const struct sv_simple_audio_format *fmt,
                        SvErrorInfo *errorOut)
{
    QBAudioStreamDesc self = NULL;
    SvErrorInfo error = NULL;

    if (!fmt) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
    } else {
        self = (QBAudioStreamDesc) SvTypeAllocateInstance(QBAudioStreamDesc_getType(), &error);
        if (self) {
            self->trackIndex = trackIndex;
            self->codec = fmt->codec;
            self->type = fmt->type;
            self->isMultiChannel = fmt->is_multichannel;
            if (fmt->lang)
                strncpy(self->lang, fmt->lang, 3);
        }
    }

    SvErrorInfoPropagate(error, errorOut);
    return self;
}

bool
QBAudioStreamDescIsMatching(QBAudioStreamDesc self,
                            const struct sv_simple_audio_format *fmt)
{
    if (!self || !fmt)
        return false;

    if (self->codec != fmt->codec)
        return false;
    if (self->type != fmt->type)
        return false;
    if ((self->isMultiChannel && !fmt->is_multichannel)
        || (!self->isMultiChannel && fmt->is_multichannel))
        return false;

    if (!fmt->lang)
        return self->lang[0] == '\0';
    return strcmp(self->lang, fmt->lang) == 0;
}

void
QBAudioStreamDescRegisterJSONSerializers(void)
{
    SvJSONClass helper = SvJSONClassCreate(saveQBAudioStreamDesc, loadQBAudioStreamDesc, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBAudioStreamDesc"), (SvGenericObject) helper, NULL);
    SVRELEASE(helper);
}

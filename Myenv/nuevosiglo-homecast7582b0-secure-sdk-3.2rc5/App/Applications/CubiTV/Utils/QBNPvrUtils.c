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

#include <NPvr/QBnPVRProvider.h>
#include <SvFoundation/SvArray.h>
#include <main.h>
#include <Utils/QBNPvrUtils.h>
#include <libintl.h>

QBnPVRRecording
QBNPvrUtilsGetRecordingOfTypeForEvent(AppGlobals appGlobals, QBnPVRRecordingType recType, SvEPGEvent event)
{
    QBnPVRRecording rec = NULL;

    SvArray recordings = SvArrayCreate(NULL);
    SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, listRecordingsByEvent, event, recordings);

    if (SvArrayCount(recordings) == 0) {
        goto fini;
    }

    if (SvArrayCount(recordings) == 1) {
        rec = (QBnPVRRecording) SvArrayAt(recordings, 0);
        goto fini;
    }

    QBnPVRRecording tmpRec;
    SvIterator it = SvArrayIterator(recordings);
    while ((tmpRec = (QBnPVRRecording) SvIteratorGetNext(&it))) {
        if (tmpRec->type == recType) {
            rec = tmpRec;
            break;
        }
    }

fini:
    SVRELEASE(recordings);
    return rec;
}

SvString
QBNPvrUtilsCreateRecordingsNumberAndQuotaSubcaption(QBActiveArray array, bool displayQuota)
{
    int count = QBActiveArrayCount(array);
    SvStringBuffer buf = SvStringBufferCreate(NULL);
    SvStringBufferAppendFormatted(buf, NULL, ngettext("%i recording", "%i recordings", count), count);

    if (displayQuota) {
        SvStringBufferAppendFormatted(buf, NULL, ", ");
        int quota = 0;
        SvIterator it = QBActiveArrayIterator(array);
        QBnPVRRecording rec;
        while ((rec = (QBnPVRRecording) SvIteratorGetNext(&it))) {
            quota += rec->quota.usedSeconds;
        }
        int quotaMinutes = quota / 60;
        int quotaHours = quotaMinutes / 60;
        if (quotaHours || !quotaMinutes) {
            SvStringBufferAppendFormatted(buf, NULL, ngettext("%i hour ", "%i hours ", quotaHours), quotaHours);
            quotaMinutes %= 60;
        }
        SvStringBufferAppendFormatted(buf, NULL, ngettext("%i minute ", "%i minutes ", quotaMinutes), quotaMinutes);
    }
    SvString subcaption = SvStringBufferCreateContentsString(buf, NULL);
    SVRELEASE(buf);
    return subcaption;
}

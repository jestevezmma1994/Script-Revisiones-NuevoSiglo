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


#include <QBSecureLogManager.h>
#include <QBSmartcard2Interface.h>
#include <QBICSmartcardInfo.h>
#include <QBICSmartcardSubscription.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvStringBuffer.h>
#include <SvFoundation/SvInterface.h>
#include <safeFiles.h>
#include "smartcardLogs.h"
#include <SvCore/SvCommonDefs.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <QBJSONUtils.h>

struct QBSmartcardLog_t {
    QBCASSmartcardState state;
    int noAccesError;
    QBCASCmd infoCmd;
    QBCASCmd subsCmd;
    SvString prefixFile;
    bool cardCorrect;
};
typedef struct QBSmartcardLog_t* QBSmartcardLog;

static QBSmartcardLog logger = NULL;

SvLocal void QBSmartcardLogGotInfo(void *self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    QBSmartcardLog self = self_;
    self->infoCmd = NULL;

    if (status != 0 || !info)
        return;

    SvString card_number = QBCASInfoGetDescriptiveCardNum(info);

    SvString prefix = SvStringCreateWithFormat("AXE: {\"chip_id\":\"%s\",\"smartcard_id\":\"%s\"} ",
            (!status && info->chip_id) ? SvStringCString(info->chip_id) : "unknown",
            (!status && card_number && self->state == QBCASSmartcardState_correct) ? SvStringCString(card_number) : "unknown");

    QBBufferToFile(SvStringCString(self->prefixFile), SvStringCString(prefix));

    SVRELEASE(prefix);
}

SvLocal void QBSmartcardLogGotSubscriptions(void *self_, const QBICSmartcardCmd* cmd, int status, SvArray subscriptions)
{
    QBSmartcardLog self = self_;
    self->subsCmd = NULL;

    if (status != 0 || subscriptions == NULL) {
        return;
    }

    SvStringBuffer buffer = SvStringBufferCreate(NULL);
    SvStringBufferAppendFormatted(buffer, NULL, "JSON:{\"no_access_event\":\"%i\",", self->noAccesError);

    if(SvArrayCount(subscriptions)) {
        SvStringBufferAppendFormatted(buffer, NULL, "\"subscriptions\":[");
        SvIterator it = SvArrayIterator(subscriptions);
        QBICSmartcardSubscription* sub;
        bool addSeparator = false;
        while((sub = (QBICSmartcardSubscription*) SvIteratorGetNext(&it))) {
            char* start = QBStringCreateJSONEscapedString(SvStringCString(sub->data[0].start_str));
            char* end = QBStringCreateJSONEscapedString(SvStringCString(sub->data[0].end_str));
            char* entitlement = QBStringCreateJSONEscapedString(SvStringCString(sub->data[0].entitlement));
            if (!addSeparator) {
                SvStringBufferAppendFormatted(buffer, NULL, "{\"start\":\"%s\",\"end\":\"%s\",\"entitlement\":\"%s\"}", start, end, entitlement);
                addSeparator = true;
            } else {
                SvStringBufferAppendFormatted(buffer, NULL, ",{\"start\":\"%s\",\"end\":\"%s\",\"entitlement\":\"%s\"}", start, end, entitlement);
            }
            free(start);
            free(end);
            free(entitlement);
        }
        SvStringBufferAppendFormatted(buffer, NULL, "]");
    } else {
        SvStringBufferAppendFormatted(buffer, NULL, "\"subscriptions\":\"none\"");
    }

    SvStringBufferAppendFormatted(buffer, NULL, "}");

    SvString text = SvStringBufferCreateContentsString(buffer, NULL);

    QBSecureLogEvent("smartcard", "Notice.Smartcard.GotSubscription", "%s", SvStringCString(text));

    SVRELEASE(text);
    SVRELEASE(buffer);
}

SvLocal void QBSmartcardLogUpdatePrefix(QBSmartcardLog self)
{
    if (self->infoCmd)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->infoCmd);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, QBSmartcardLogGotInfo, self, &self->infoCmd);
}

SvLocal void QBSmartcardLogStateChanged(void *self_, QBCASSmartcardState state)
{
    QBSmartcardLog self = self_;
    self->state = state;

    if (state == QBCASSmartcardState_removed) {
        // Notify about removing only when SmartCard was identified as correct one.
        // It protects from sending incorrect notification when box is started because then
        // first call of this callback notifies that state == QBCASSmartcardState_removed
        // even when SmartCard is inserted in slot.
        // And also we will notice about removing Smart Card only when notification about insertion was
        // previously sent.
        if (self->cardCorrect) {
            QBSecureLogEvent("smartcard", "Notice.Smartcard.StatusChanged", "JSON:{\"status\":\"removed\"}");
        }

        self->cardCorrect = false;
    } else if (state == QBCASSmartcardState_correct) {
        // Notify about insertion only when SmartCard state is correct.
        QBSecureLogEvent("smartcard", "Notice.Smartcard.StatusChanged", "JSON:{\"status\":\"inserted\"}");

        self->cardCorrect = true;
    }

    QBSmartcardLogUpdatePrefix(self);
}

SvLocal void QBSmartcardLogEcmStatus(void* self_, uint8_t sessionId, const int16_t pid,
                                     QBCASEcmStatus status,
                                     bool is_extra_ecm, int no_access_code,
                                     const QBDescramblerCW* cw)
{
    if (!SvPlayerManagerIsPlaybackSession(SvPlayerManagerGetInstance(), sessionId))
        return;

    QBSmartcardLog self = self_;
    if (status == QBCASEcmStatus_failed && !self->subsCmd) {
        self->noAccesError = no_access_code;

        if (SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
            SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getSubscriptions, QBSmartcardLogGotSubscriptions, self, &self->subsCmd);
        }
    }
}

void QBSmartcardLogCreate(SvString prefixFile)
{
    logger = calloc(1, sizeof(struct QBSmartcardLog_t));
    logger->state = QBCASSmartcardState_removed;
    logger->prefixFile = SVRETAIN(prefixFile);
}

static const struct QBCASCallbacks_s smartcard_callbacks = {
    .smartcard_state = QBSmartcardLogStateChanged,
    .ecm_status      = QBSmartcardLogEcmStatus,
};


void QBSmartcardLogStart(void)
{
    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks)&smartcard_callbacks, logger, "Smartcard logs");
    QBSmartcardLogUpdatePrefix(logger);
}

void QBSmartcardLogStop(void)
{
    if (logger->subsCmd)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, logger->subsCmd);
    if (logger->infoCmd)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, logger->infoCmd);

    logger->subsCmd = logger->infoCmd = NULL;

    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, (QBCASCallbacks)&smartcard_callbacks, logger);
}

void QBSmartcardLogDestroy(void)
{
    SVRELEASE(logger->prefixFile);
    free(logger);
}

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_SMARTCARD2_INTERFACE_H
#define QB_SMARTCARD2_INTERFACE_H

/**
 * @file QBSmartcard2Interface.h QBSmartcard2Interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBSmartcard2InterfaceCallbacks.h>
#include <QBICSmartcardUserText.h>
#include <QBICSmartcardMail.h>
#include <QBICSmartcardFingerprint.h>
#include <QBICSmartcardEncrypt.h>
#include <QBICSmartcardDriverVersion.h>
#include <QBICSmartcardOption.h>
#include <QBCAS.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvArray.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBSmartcard2
 * @ingroup DRM
 * @{
 **/

typedef enum QBICSmartcardPinStatus_e {
    QBICSmartcardPinStatus_ok = 0,
    QBICSmartcardPinStatus_invalid = 1,
    QBICSmartcardPinStatus_error = -1,
} QBICSmartcardPinStatus;

typedef enum QBICSmartcardPinUsage_e {
    QBICSmartcardPinUsage_normal,
    QBICSmartcardPinUsage_maturity_rating,
    QBICSmartcardPinUsage_event,
    QBICSmartcardPinUsage_token_debit,
} QBICSmartcardPinUsage;

struct QBICSmartcardCmd_s;
typedef struct QBICSmartcardCmd_s QBICSmartcardCmd;
typedef void (QBICSmartcardCallbackFun)(void *arg, const QBICSmartcardCmd *cmd, int status);
typedef void (QBICSmartcardCallbackArrayFun)(void *arg, const QBICSmartcardCmd *cmd, int status, SvArray a);
typedef void (QBICSmartcardCallbackObjFun)(void *arg, const QBICSmartcardCmd *cmd, int status, SvObject info);

typedef struct QBSmartcard2Interface_ {
    /**
     * Called while ECM is not connected with a content e.g. ECM from PlayReady individualization
     */
    int (*pushSpecialECM)(SvObject self_, QBICSmartcardCallbackFun* fun, void* arg, QBCASCmd* cmd_out, SvData ecmSection);

    /**
     * Gets subscriptions from smartcard
     */
    int (*getSubscriptions)(SvObject self_, QBICSmartcardCallbackArrayFun* fun, void* arg, QBCASCmd* cmd_out);

    /**
     * Gets events from smartcard
     */
    int (*getEvents)(SvObject self_, QBICSmartcardCallbackArrayFun* fun, void* arg, QBCASCmd* cmd_out);

    /**
     * Gets purses from smartcard
     */
    int (*getPurses)(SvObject self_, QBICSmartcardCallbackArrayFun* fun, void* arg, QBCASCmd* cmd_out);

    /**
     * Gets purse status from smartcard
     */
    int (*getPurseStatus)(SvObject self_, QBICSmartcardCallbackObjFun* fun, void* arg, QBCASCmd* cmd_out,
                          int purse_ref, bool debits, bool credits);

    /**
     * Marks user text (internal counter on related to this user text should be decrement)
     */
    int (*markUserText)(SvObject self_, struct QBICSmartcardUserText_s* userText, QBICSmartcardMarkUserTextType type);

    /**
     * Marks mail (internal counter on related to this mail should be decrement)
     */
    int (*markMail)(SvObject self_, struct QBICSmartcardMail_s* mail);

    /**
     * Callback with all mails should be triggered
     */
    int (*reportAllMails)(SvObject self_);

    /**
     * Marks fingerprint as shown
     */
    int (*markFingerprint)(SvObject self_, QBICSmartcardFingerprint *fingerprint);

    /**
     * get driver information (i.e. Conax version, QBConax2 API version)
     */
    QBICSmartcardDriverVersion* (*getDriverVersion)(SvObject self_);

    int (*options)(SvObject self_, QBICSmartcardCallbackObjFun* fun, void* arg, QBCASCmd* cmd_out, enum QBICSmartcardOption_e option);

    /**
     * @deprecated encrypt method is deprecated use QBCrypto instead.
     */
    int (*encrypt)(SvObject self_, QBICSmartcardCmdEncrypt data);

    /**
     * check smartcard PIN for given type
     */
    int (*enterPin)(SvObject self_, QBICSmartcardCallbackFun* fun, void* arg, QBCASCmd* cmd_out,
                    int8_t sessionId, const unsigned char* currPin, QBICSmartcardPinUsage usage);

    /**
     * Change Smartcard PIN
     */
    int (*changePin)(SvObject self_, QBICSmartcardCallbackFun* fun, void* arg, QBCASCmd* cmd_out,
                     const unsigned char* currPin, const unsigned char* newPin);

    /**
     * Trigger QBICSmartcard to check if caches PIN is a valid one
     */
    int (*checkCachedPin)(SvObject self_);

    /**
     * Set new maturity rating level
     */
    int (*changeMaturityRating)(SvObject self_, QBICSmartcardCallbackFun* fun, void* arg, QBCASCmd* cmd_out,
                                const unsigned char* currPin, int newLevel);

    /**
     * Get current maturity rating
     */
    int (*getMaturityRating)(SvObject self_);

    /**
     * Registers Conax specific callbacks
     */
    void (*addConaxCallbacks)(SvObject self_, const struct QBICSmartcardCallbacks_s* callbacks, void* target, const char* name);

    /**
     * Unregister Conax specific callbacks
     */
    void (*removeConaxCallbacks)(SvObject self_, const struct QBICSmartcardCallbacks_s* callbacks, void* target);
} *QBSmartcard2Interface;

SvInterface QBSmartcard2Interface_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif


#endif // #ifndef QB_SMARTCARD2_INTERFACE_H

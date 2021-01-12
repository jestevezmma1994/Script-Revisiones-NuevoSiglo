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

#ifndef QB_SMARTCARD2_INTERFACE_CALLBACKS_H
#define QB_SMARTCARD2_INTERFACE_CALLBACKS_H

/**
 * @file QBSmartcard2InterfaceCallbacks.h QBSmartcard2InterfaceCallbacks
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBICSmartcardEncrypt.h>
#include <QBICSmartcardSessionDescription.h>
#include <SvFoundation/SvObject.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBSmartcard2
 * @ingroup DRM
 * @{
 **/

typedef enum QBICSmartcardNoAccessEvent_e {
    // each enum must be negative.
    QBICSmartcardNoAccessEvent_geographical_blackout = -1,
    QBICSmartcardNoAccessEvent_no_access_to_network = -2,
} QBICSmartcardNoAccessEvent;

struct QBICSmartcardCallbacks_s {
    void (*smartcard_state_report)(void *target);

    void (*fingerprint)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);    /// QBICSmartcardFingerprint
    void (*user_text)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);     /// QBICSmartcardUserText
    void (*mail)(void *target, SvObject obj);                            /// QBICSmartcardMail
    void (*message_removed)(void *target, int slot);

    /// event : QBICSmartcardNoAccessEvent
    void (*no_access_event)(void *target, const QBICSmartcardSessionDescription *sessionDesc, int event);

    void (*high_rating)(void *target, const QBICSmartcardSessionDescription *sessionDesc, int current, int needed);
    void (*accept_viewing)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);     /// QBICSmartcardAcceptViewing
    void (*token_debit)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);        /// QBICSmartcardTokenDebit
    void (*order_info)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);         /// QBICSmartcardOrderInfo

    /// @deprecated This callback is deprecated use QBCrypto instead.
    void (*data_encrypted)(void *target, QBICSmartcardCmdEncrypt crypt);

    // sessionId with value -1 means that current apdu isn't form session oriented message
    // apdus from ECM should have a valid sessionId
    void (*apdu_data)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);          /// QBICSmartcardAPDUData

    void (*driver_version)(void *target, SvObject obj);      /// QBICSmartcardDriverVersion

    void (*decryption_state)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);    /// QBICSmartcardDecryptionState

    void (*uri_data)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);            /// QBICSmartcardURIData

    void (*cached_pin_status)(void *target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj);   /// QBICSmartcardCachedPIN

    // TODO: other, as needed
};

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif


#endif // #ifndef QB_SMARTCARD2_INTERFACE_CALLBACKS_H

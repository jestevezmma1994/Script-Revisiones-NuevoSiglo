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
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_CONAX_API_H
#define QB_CONAX_API_H

#include "structures.h"

#ifdef __cplusplus__
extern "C" {
#endif

/**
 * @file QBConax/api.h
 * @brief Cubiware Conax module functions.
 **/

/**
 * @defgroup QBConax Cubiware Conax API
 * @{
 *
 * @brief QBConax API for communicating with the low-level module implementation.
 *
 * @details
 *
 * The module handles the following areas:
 * @li smartcard state management
 * @li smartcard low-level communication
 * @li smartcard request building, and response parsing for queries in this API
 * @li keeping and updating nvram storage for keys (chipset-pairing and link-protection) and host-data (fp, host-message, mail)
 * @li decrypting link-protected fields in smartcard responses
 * @li translating smartcard response into proper chipset-pairing error codes
 *
 * All functions will be called from a single thread.
 * @ref QBConaxGetNotification() function will be polling periodically (about 50 times per second).
 * All other functions will be called as needed:
 * @li by the UI commands (displaying menus, entering pin, getting card info),
 * @li by the stream SI (CAT, EMM, ECM, PMT) changes.
 *
 * Every card-related function shall return an error if the card has incorrect state.
 *
 * Every function shall block until the card/module is ready to process the given command (if card's state is correct).
 * @ref QBConaxPushECM() is an exception. It shall only be handled when we can start the card communication immediately.
 * The reason is that we are allowed to drop an ECM when a newer version is received.
 *
 * After the module accepts the ECM to be handled, it can choose to return immediately, or after the ECM is completely handled.
 * This includes:
 * @li card communication error (e.g. card removed in the meantime)
 * @li the ECM was rejected by the card (invalid data), and so the card had to be reset
 * @li the ECM was accepted by the card - then the module should notify every part of smartcard response
 *     Errors in smartcard response shall be detected by the module, and reported with notifications.
 *     Parsing ECM response should be finished with @ref QBConaxNotification_ecm_finished
 *
 * If Conax6.0 will be used 'text' attribute inside 'QBConaxCmdUserText' shouldn't be just a simple copy from Smartcard but
 * some translation should be done. Read more in file 'structures.h' inside 'QBConaxCmdUserText' structure.
 *
 **/

/// API VERSION
#define QB_CONAX2__API_VERSION 0x00020005 // version 2.05

//------------------------------
// init
//------------------------------


/** Initialize the module.
 *  Reset all internal state.
 *  Start smartcard communication, according to Conax specs.
 *  It the module was not deinitialized from the previous run, it must be first deinitialized automatically.
 *  Should send notification about current smartcard state (if no card, should send "removed").
 *  Additionally notification with all keys already stored on the flash should be sent.
 *  Module shouldn't reset any session, a proper reset will be done by application by calling
 *   QBConaxResetSession(sessionId)
 * @return  0 on success, negative on error
 */
int QBConaxInit(void);

/** Elegantly release all internal resources, and stop communication with the smartcard.
 * @return  0 on success, negative on error
 */
int QBConaxDeinit(void);


//------------------------------
// getting info
//------------------------------

/** Get chipset/module information.
 *  Must be always available.
 * @param[out] info  info to be filled by this function
 * @return  0 on success, negative on error
 */
int QBConaxGetHostInfo(struct QBConaxCmdHostInfo* info);

/** Get smartcard information.
 *  Smartcard info should be available when the card is "correct".
 * @param[out] info  info to be filled by this function
 * @return  0 on success, negative on error
 */
int QBConaxGetCardInfo(struct QBConaxCmdCardInfo* info);

/** Get amount of subscription entries from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[out] count  total amount of subscriptions
 * @return  0 on success, negative on error
 */
int QBConaxGetSubscriptionCount(int32_t* count);

/** Get info about subscription entry @a info::index from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[in,out] info  to be filled, according to @a info::index
 * @return  0 on success, negative on error
 */
int QBConaxGetSubscriptionInfo(struct QBConaxCmdSubscriptionInfo* info);

/** Get amount of event entries from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[out] count  total amount of events
 * @return  0 on success, negative on error
 */
int QBConaxGetEventCount(int32_t* count);

/** Get info about event entry @a info::index from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[in,out] info  to be filled, according to @a info::index
 * @return  0 on success, negative on error
 */
int QBConaxGetEventInfo(struct QBConaxCmdEventInfo* info);

/** Get amount of purse entries from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[out] count  total amount of purses
 * @return  0 on success, negative on error
 */
int QBConaxGetPurseCount(int32_t* count);

/** Get info about purse entry @a info::index from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[in,out] info  to be filled, according to @a info::index
 * @return  0 on success, negative on error
 */
int QBConaxGetPurseInfo(struct QBConaxCmdPurseInfo* info);

/** Get amount of debit entries of purse @a purseId from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[in] purseId  id of the purse, as taken from @ref QBConaxGetPurseInfo
 * @param[out] count  total amount of debit entries on purse @a purseId
 * @return  0 on success, negative on error
 */
int QBConaxGetDebitCount(uint32_t purseId, int32_t* count);

/** Get info about purse's @a info::purseId debit entry @a info::index from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[in,out] info  to be filled, according to @a info::index, and @a info::purseId, as taken from @ref QBConaxGetPurseInfo
 * @return  0 on success, negative on error
 */
int QBConaxGetDebitInfo(struct QBConaxCmdDebitInfo* info);

/** Get amount of credit entries of purse @a purseId from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[in] purseId  id of the purse, as taken from @ref QBConaxGetPurseInfo
 * @param[out] count  total amount of credit entries on purse @a purseId
 * @return  0 on success, negative on error
 */
int QBConaxGetCreditCount(uint32_t purseId, int32_t* count);

/** Get info about purse's @a info::purseId credit entry @a info::index from the smartcard.
 *  Should be available when the card is "correct", otherwise can fail with an error.
 * @param[in,out] info  to be filled, according to @a info::index, and @a info::purseId, as taken from @ref QBConaxGetPurseInfo
 * @return  0 on success, negative on error
 */
int QBConaxGetCreditInfo(struct QBConaxCmdCreditInfo* info);


//------------------------------
// pin
//------------------------------

/** Verify if pin code @a code is correct, and unblock given functionality as given in @a type.
 * @param[in] sessionId  see conax specs
 * @param[in] type  type of functionality to unblock, see @ref QBConaxPINType
 * @param[in] code  pin code, to verify the request by the card, null-terminated array of 4 ascii digits
 * @return  negative on smartcard communication error, else value from @ref QBConaxPINStatus
 */
int QBConaxEnterPin(uint8_t sessionId, enum QBConaxPINType type, const uint8_t* code);

/** Change old pin to new one.
 * @param[in] codeOld  old pin code, to verify the request by the card, null-terminated array of 4 ascii digits
 * @param[in] codeNew  new pin code, null-terminated array of 4 ascii digits
 * @return  negative on smartcard communication error, else value from @ref QBConaxPINStatus
 */
int QBConaxChangePin(const uint8_t* codeOld, const uint8_t* codeNew);

/** Change allowed maturity rating on the card.
 * @param[in] code  pin code, to verify the request by the card, null-terminated array of 4 ascii digits
 * @param[in] newRating  rating to set on the card
 * @return  negative on smartcard communication error, else value from @ref QBConaxPINStatus
 */
int QBConaxChangeMaturityRating(const uint8_t* code, uint8_t newRating);


//------------------------------
// user text
//------------------------------

/** Get info about user text @a cmd::slotIndex from the NVRAM.
 *  Must be available always.
 * @param[in,out] cmd  to be filled, according to @a cmd::slotIndex
 * @return  0 on success, negative on error
 */
int QBConaxGetUserText(struct QBConaxCmdUserText* cmd);

/** Mark the user-text as used once, increasing its read counter, as seen in @ref QBConaxCmdUserText.
 * @param[in] slotIndex  index of the message to mark, 16 most significant bits describe why application marked that user test (see QBConaxMarkUserTextType)
 * @return  0 on success, negative on error
 */
int QBConaxMarkUserText(uint32_t slotIndex);

/** Get info about fingerprint from the NVRAM.
 *  Must be available always.
 * @param[out] cmd  to be filled
 * @return  0 on success, negative on error
 */
int QBConaxGetFingerprint(struct QBConaxCmdFingerprint* cmd);

/** Mark the fingerprint as used
 * @return  0 on success, negative on error
 */
int QBConaxMarkFingerprint(struct QBConaxCmdFingerprint* cmd)  __attribute__((weak));

//------------------------------
// descrambling
//------------------------------

/** (deprecated - session reset after changed scrambled => FTA will be done by calling QBConaxResetSession())
 * Notify that the program on session @a sessionId has changed (e.g. from scrambled to FTA).
 *  The smartcard is detecting some changes in the stream by looking at ECMs it receives,
 *  but we need to tell it when we go to FTA.
 * @param[in] sessionId  see conax specs
 * @param[in] isScrambled  true iff the session will now process scrambled stream
 * @return  0 on success, negative on error
 */
int QBConaxUpdateScrambledStatus(uint8_t sessionId, uint8_t isScrambled);

/** Pass the current scrambling mode, as found in the PMT.
 *  The usual use-case for the module is to update pairing mechanism on the card.
 * @param[in] sessionId  see conax specs
 * @param[in] mode (from scrambling_descriptor) as found in the PMT (@c 0 iff unknown)
 *            while mode 0x70-0x7f then it's AES-IDSA-ATIS, else it's assumed to be DVB-CSA
 * @return  0 on success, negative on error
 */
int QBConaxUpdateScramblingMode(uint8_t sessionId, uint8_t mode);

/** Module/card should start processing given ECM.
    The result should be delivered asynchronously, with @ref QBConaxGetNotification.
    The end of ECM processing must be marked with one of the following notifications:
    \li any notification about changed smartcard state
    \li @ref QBConaxNotification_ecm_finished

    During ECM processing, the module has to send all the info returned by the card from the ECM response:
    \li any notification about no-access (including pairing/maturity/token/order/view/blackout, etc.)
    \li @ref QBConaxNotification_control_word
    \li @ref QBConaxNotification_fingerprint

   @param[in] cmd @ref QBConaxCmdPushECM
   @return  0 on success, negative on error (invalid section, invalid state)
 */
int QBConaxPushECM(struct QBConaxCmdPushECM* cmd);


//------------------------------
// cat/emm
//------------------------------

/** Process CAT section, and return EMM filters.
 * @param[in,out] cmd  on input, CAT section, on output, EMM filters
 * @return  0 on success, negative on error
 */
int QBConaxParseCAT(struct QBConaxCmdParseCAT* cmd);

/** Process EMM section.
 * @param[in] pid  pid that the section came from, -1 if not from DVB
 * @param[in] section  EMM section bytes
 * @param[in] len  EMM section length
 * @return  0 on success, negative on error
 */
int QBConaxPushEMM(int pid, const uint8_t* section, int len);

//------------------------------
// notifications
//------------------------------

/** Poll for notifications from the module.
 *  Must never block, never wait for an event with timeout, etc.
 *  This function will be called frequently (about 20 times per second), but from the same thread as other functions.
 * @param[out] cmd  notification type and data to be filled out by the module
 * @return  0 on success, negative on error
 */
int QBConaxGetNotification(struct QBConaxCmdNotification* cmd);


//------------------------------
// misc
//------------------------------

/** (deprecated) Application will be responsible for a session resetting
 * Put the smartcard in requested standby state.
 * @param[in] state  requested standby state
 * @return  0 on success, negative on error
 */
int QBConaxSetStandby(enum QBConaxStandbyState state);


/** Get/Set option, module specific information look at enum QBConaxOption_e
 * It is required for every new module.
 * @param[in, out] cmd to be filled out by the module
 * @return  0 on success, negative on error
 */
int QBConaxOptions(struct QBConaxCmdOptions* cmd) __attribute__((weak));

/** Reset session for a given sessionId
 * This will be called after QBConaxInit, after inserting new card,
 * after exiting from standby mode. Reset only when card is correct.
 * Module should not reset any session automaticaly, application is responsible for it.
 * @param[in] sessionId  see conax specs
 * @return  0 on success, negative on error
 */
int QBConaxResetSession(uint8_t sessionId);

//------------------------------
// Encrypting / Decrypting data
//------------------------------

/**
 * Encrypting custom data
 * @param[in] cmd       @ref QBConaxCmdEncrypt
 * @return              0 on success, -1 on failure
 */
int QBConaxEncrypt(struct QBConaxCmdEncrypt *cmd) __attribute__((weak));

/**
 * @}
 **/

#ifdef __cplusplus__
}
#endif

#endif // #ifndef QB_CONAX_H

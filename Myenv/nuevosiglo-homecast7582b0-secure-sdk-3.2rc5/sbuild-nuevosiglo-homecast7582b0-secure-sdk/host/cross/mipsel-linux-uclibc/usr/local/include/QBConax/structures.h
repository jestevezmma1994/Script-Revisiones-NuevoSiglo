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

#ifndef QB_CONAX_STRUCTURES_H
#define QB_CONAX_STRUCTURES_H

#ifdef MODULE
# include <linux/kernel.h>
#else
# include <stdint.h>
#endif

#include <QBDescrambler/basic_types.h>

/**
 * @file QBConax/structures.h
 * @brief Data types used by QBConax api.
 **/

/**
 * @ingroup QBConax
 * @{
 **/

//------------------------------
// misc
//------------------------------

/// Date class
struct QBConaxDate
{
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
};

/// Time class
struct QBConaxTime
{
  uint8_t  hour;
  uint8_t  minute;
};

/// Units that Event cost is expressed in.
enum QBConaxUnitsType
{
  QBConaxUnitsType_none     = 0, ///< unlimited access, no units
  QBConaxUnitsType_minutes  = 1, ///< cost is in minutes
  QBConaxUnitsType_credits  = 2, ///< cost is in credits
};

/// Functionality to unblock by sendign pin to the smartcard for verification.
/// Those values cannot be changed, since they are the same as in Conax specs.
enum QBConaxPINType
{
  QBConaxPINType_generic  = 0, ///< just verify if given pin is correct
  QBConaxPINType_maturity = 1, ///< verify the pin, and override maturity lock
  QBConaxPINType_event    = 2, ///< verify the pin, and accept "accept-viewing" info
  QBConaxPINType_tokens   = 3, ///< verify the pin, and accept "token-debit" info
};

/// Return value of pin verification functions.
enum QBConaxPINStatus
{
  QBConaxPINStatus_error = -1,  ///< an error has occurred
  QBConaxPINStatus_ok = 0,      ///< entered pin is correct
  QBConaxPINStatus_invalid = 1, ///< entered pin is incorrect
};


//------------------------------
// Card Info
//------------------------------

/// Generic info about the h/w and the driver.
struct QBConaxCmdHostInfo
{
  char     softwareVersion[32]; ///< module's software version
  char     chipId[16];          ///< Chip ID, null-terminated ascii string, format: "006 6460 6560"

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

/// Generic info from the card.
struct QBConaxCmdCardInfo
{
  uint8_t  casVersion;        ///< see Conax specs
  uint8_t  maxSessionCount;   ///< see Conax specs
  uint16_t countryCode;       ///< see Conax specs

  uint32_t cardNumber;        ///< zero if unknown
  uint32_t cardNumberMinor;   ///< the number after '-'
  uint16_t systemId;          ///< see Conax specs (e.g. 0xb00)

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// Subscriptions
//------------------------------

// QBConaxCmdSubscriptionCount == QBConaxCmdCount

/// Status of single subscription entry, as displayed in Conax menu.
struct QBConaxCmdSubscriptionInfo
{
  uint8_t  index;       ///< [in] index of entry to display

  char     label[128];  ///< [out] null-terminated ascii string

  /// [out] current period, and previous period
  struct {
    struct QBConaxDate  startEnd[2];
    char  entitlement[16]; ///< null-terminated ascii string
  } periods[2];

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// Events
//------------------------------

// QBConaxCmdEventCount == QBConaxCmdCount

/// Status of single event entry, as displayed in Conax menu.
struct QBConaxCmdEventInfo
{
  // [in]
  uint8_t  index;

  // [out]
  //uint32_t  tag;
  char     label[128]; ///< null-terminated ascii string
  struct {
    struct QBConaxDate  date;
    struct QBConaxTime  time;
  } startEnd[2];
  enum QBConaxUnitsType  unitsType;
  uint32_t  unitsLeft; ///< unused if @a unitsType is @ref QBConaxUnitsType_none

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// Purses (tokens)
//------------------------------

// QBConaxCmdPurseCount == QBConaxCmdCount

/// Status of single purse entry, as displayed in Conax menu.
struct QBConaxCmdPurseInfo
{
  // [in]
  uint8_t  index;

  // [out]
  uint32_t  purseId;
  char      label[128]; ///< null-terminated ascii string
  uint32_t  tokens;

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// Purse (debits)
//------------------------------

// QBConaxCmdDebitCount == QBConaxCmdCount(id=purseId)

/// Status of single debit entry, as displayed in Conax menu.
struct QBConaxCmdDebitInfo
{
  // [in]
  uint32_t  purseId;
  uint8_t  index;

  // [out]
  //uint32_t  tag;
  char      label[128]; ///< null-terminated ascii string
  uint32_t  tokens;

  struct QBConaxDate  lastAccessDate;
  struct QBConaxTime  lastAccessTime;

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// Purse (credits)
//------------------------------

// QBConaxCmdCreditCount == QBConaxCmdCount(id=purseId)

/// Status of single debit entry, as displayed in Conax menu.
struct QBConaxCmdCreditInfo
{
  // [in]
  uint32_t  purseId;
  uint8_t   index;

  // [out]
  char      label[128]; ///< null-terminated ascii string
  uint32_t  tokens;

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};


//------------------------------
// User Text
//------------------------------

// QBConaxCmdUserTextMark == QBConaxCmd(id=slotIndex)

/// Structure for returning host-message information.
struct QBConaxCmdUserText
{
  // [in]
  uint8_t  slotIndex;     ///< slot that we query the info from

  // [out]
  uint8_t  isValid;       ///< zero iff there is no user text

  // [out]
  uint8_t  isMail;        ///< non-zero for mail, zero for user-text
  uint8_t  readCounter;   ///<
  struct QBConaxDate  startDate;  ///<
  struct QBConaxTime  startTime;  ///<

  uint16_t  showDuration; ///< in seconds
  uint16_t  intervalDuration; ///< in seconds
  uint8_t   repeatCnt;    ///<

  // For Conax6.0 (i.e. QBConaxOption_cxVersion__6_0) 'text' could have a additional first byte which indicates current character encoding.
  // Conax sends sometimes first byte with 0x00 (latin table 0), but for us it is a null termination mark.
  // We want to have a behavior compatible with 'ETSI EN 300 468" (i.e. DVB standard).
  // What need to be done is to do that translation on the first byte:
  //
  // from Smartcard | DVB/Cubiware                   |          DESCRIPTION
  // ---------------------------------------------------------------------------------------------
  // 0x00          ->  remove first byte (i.e. 0x00) | (latin table 0 should be used as a default) - first byte should be removed
  // 0x01          ->   0x01                         | don't need to do anything
  // 0x05          ->   0x05                         | don't need to do anything
  // >= 0x20       ->  append 0x05 as a first byte   | this is a default for SC, so byte with '0x05' should be appended before Conax string
  //
  // maximum theoretical text size if 244 so always should be possible to add one byte if necessary
  char     text[256];     ///< null-terminated ascii string

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

// only 16 most significant bits are used, lower ones are carrying slot number
// this enum is used as a mask for slotIndex in QBConaxMarkUserText()
enum QBConaxMarkUserTextType {
    QBConaxMarkUserTextType_message_shown   = 0x00000000, ///< message was shown correctly
    QBConaxMarkUserTextType_message_dropped = 0x00010000, ///< message was dropped, in case of immediate message module should send the same message again
};

//------------------------------
// Fingerprint
//------------------------------

/// Structure for returning fingerprint information.
struct QBConaxCmdFingerprint
{
  /// [out]
  uint8_t  isValid;   ///< zero iff there is no fingerprint

  /// [out]
  int32_t  startTime; ///< seconds from last midnight or -1 for immediate
  uint32_t  duration; ///< in milliseconds

  uint16_t  posX;     ///< on-screen position from left side
  uint16_t  posY;     ///< on-screen position from top side
  uint8_t  fontSize;  ///< font size, in pixels
  char     text[256]; ///< null-terminated ascii string

  // [out], must be used only in ioctl layer
  int32_t  errorOut;

  int64_t receivingTime; ///< (from API 2.05) fingerprint receiving time in microseconds, fingerprint after receiving is valid only 24h, used only while scheduled fingerpring
  uint8_t priority;      ///< (from API 2.05) fingerprint priority level, value other than zero means that fingerprint has priority.
};

//-----------------------------------
// Get Options
//-----------------------------------

//**************************** AVAILABLE OPTIONS ****************************//
enum QBConaxOption_e {
/// should return current api version (i.e. QB_CONAX2__API_VERSION from api.h used to compilation)
/// response in u.apiVersion
  QBConaxOption_get_api_version              = 0,

/// Get information about supported session count implemented in the module.
/// Should return 1 if module don't support multi-sessions or 'n>1' in the other way
/// This value should be available without any card inserted. It isn't connected with a card sessions number.
/// response in u.supportedSessionsCount
  QBConaxOption_get_supported_sessions_count = 1,

/// used by some modules to implement One Time Programming
  QBConaxOption_set_OTP                      = 2,

/// get information which conax specification is implemented
/// i.e. 5.0a, 6.0 or other
/// response in u.conaxVersion
  QBConaxOption_get_conax_version            = 3,
};

enum QBConaxCxVersion_e {
/// response from u.conaxVersion
  QBConaxOption_cxVersion__unknown          = 0,
  QBConaxOption_cxVersion__5_0_a            = 1,
  QBConaxOption_cxVersion__6_0              = 2,
};

/// Structure for get/set module options
struct QBConaxCmdOptions
{
  /// [in]
  uint32_t option;        ///< type QBConaxOption_e

  /// [out]
  union {
    uint32_t apiVersion;
    uint8_t supportedSessionsCount;
    uint32_t conaxVersion;  ///< type QBConaxCxVersion_e
    uint32_t padding[64];
  } u;

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};


//------------------------------
// ECM
//------------------------------

/// Type of result of function call @ref QBConaxPushECM.
enum QBConaxECMError
{
  QBConaxECMError_ok      = 0, ///< ECM is to be sent to the card immediately
  QBConaxECMError_busy    = 1, ///< module/card is busy, cannot push the ECM right now
  QBConaxECMError_invalid = 2, ///< the ECM section is invalid
};

/// ECM flags
#define QBCONAX_ECM_MODE_MASK 0x01 ///< set to '1' for 'query' mode, '0' if in 'operational' mode

/// Structure for passing arguments to function call @ref QBConaxPushECM.
struct QBConaxCmdPushECM
{
  /// [in]
  uint8_t  section[256];  ///< ecm section data
  uint16_t len;           ///< ecm section length

  uint8_t  sessionId;     ///< sessionId, as in Conax specs

  uint8_t  flags;         ///< flags (e.g. 'operational/query' mode)

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// CAT/EMM
//------------------------------

/// Description of a single EMM filter.
/// @see QBConaxParseCAT
/// @see QBConaxCmdParseCAT
struct QBConaxEMMFilter
{
  uint16_t pid;
  uint8_t  data[16]; ///< without tableId, and without sectionLen, e.g.: "00 00 00 7b 6d f6 8a", or "00 00 00 00 3d b6 fb"
  uint8_t  mask[16]; ///< the same rules apply as for @a data
};

/// Structure for passing arguments to function call @ref QBConaxParseCAT.
struct QBConaxCmdParseCAT
{
  /// [in]
  uint32_t len;
  uint8_t  section[1024];

  /// [out]
  uint32_t  filterCnt;
  struct QBConaxEMMFilter  filters[5];

  /// [out], must be used only in ioctl layer
  int32_t  errorOut;
};


//------------------------------
// Standby
//------------------------------

/// Type of standby state the card can be put into.
enum QBConaxStandbyState
{
  QBConaxStandbyState_off     = 0, ///< no standby, normal operation
  QBConaxStandbyState_active  = 1, ///< active standby, see Conax specs
  QBConaxStandbyState_passive = 2, ///< passive standby, see Conax specs
};


//-----------------------------------
// Notifications - Order
//-----------------------------------

/// Data notified with @ref QBConaxNotification_order
struct QBConaxNotificationOrder
{
  //uint32_t  productId;
  char    price[128];
  char    label[256];
};

//-----------------------------------
// Notifications - Accept Viewing
//-----------------------------------

/// Data notified with @ref QBConaxNotification_accept_viewing
struct QBConaxNotificationAcceptViewing
{
  //uint32_t  productId;
  uint16_t  minutes;
  char      label[256];
};

//-----------------------------------
// Notifications - Token Debit
//-----------------------------------

/// Data notified with @ref QBConaxNotification_token_debit
struct QBConaxNotificationTokenDebit
{
  uint32_t  purseId;
  //uint32_t  eventTag;
  uint32_t tokenCost;
  uint8_t  isPPM;           ///< non-zero if pay-per-minute
  uint8_t  hasEnoughTokens; ///< zero if unable to play due to insufficient tokens
  char     label[256];
};

//-----------------------------------
// Notifications - Maturity Rating
//-----------------------------------

/// Data notified with @ref QBConaxNotification_maturity_rating
struct QBConaxNotificationMaturityRating
{
  uint8_t  currentRating; ///< rating the card allows to play
  uint8_t  wantedRating;  ///< rating needed by the content
};

//-----------------------------------
// Notifications - Correct Card
//-----------------------------------

/// Data notified with @ref QBConaxNotification_card_state_correct
struct QBConaxNotificationCorrectCard
{
  uint32_t  cardNumber;     ///< @deprecated not used anymore, use @ref QBConaxGetCardInfo instead
  uint32_t  cardNumberMinor;///< @depracated  not used anymore, use @ref QBConaxGetCardInfo instead
  uint16_t  systemId;       ///< card's systemId, e.g. 0xb00
  uint8_t   currentRating;  ///< card's maturity rating, just as returned in @ref QBConaxNotificationMaturityRating
};

//-----------------------------------
// Notifications - Pairing Changed
//-----------------------------------

/// pairing change type
typedef enum
{
  QBConaxPairingChangeType_keyAdded    = 0, ///< self explained
  QBConaxPairingChangeType_keyRemoved  = 1, ///< self explained
  QBConaxPairingChangeType_keyChanged  = 2, ///< self explained
  QBConaxPairingChangeType_keyInitial  = 3, ///< (already on flash after reboot)
} QBConaxPairingChangeType;

/// Data notified with @ref QBConaxNotification_pairing_changed
/// After QBConaxInit() module should send notification with all keys already stored on the flash
/// After each keystore change a proper notification should be also sent
struct QBConaxNotificationPairingChanged
{
  uint32_t keyType;     ///< type of the key (e.g. CPK, LPK, see conax specs for proper values)
  uint32_t keyId;       ///< key id
  uint8_t  changeType;  ///< @ref QBConaxPairingChangeType
  uint8_t  keyLen;      ///< length of valid data in @a keyData buffer
  uint8_t  keyData[32]; ///< key's data, up to @a keyLen bytes are valid
};

//-----------------------------------
// Notifications - Custom Data
//-----------------------------------

#define QBConaxCustomDataMaxLen  (4*1024)

/// Data notified with @ref QBConaxNotification_custom_data
struct QBConaxNotificationCustomData
{
  uint32_t type;                          ///< type of this custom message (implementation dependent)
  uint32_t len;                           ///< length of valid data in @a data buffer
  uint8_t  data[QBConaxCustomDataMaxLen]; ///< custom message's data, up to @a len bytes are valid
};

//-----------------------------------
// Notifications - Encrypt/Decrypt Data
//-----------------------------------

struct QBConaxCmdEncrypt
{
  uint8_t encrypt;        ///< 0 -> means encrypt data, 1-> means decrypt data
  uint8_t cookie;         ///< the resulting notification structure has to have the same cookie.
  uint8_t data[1024];     ///< data to encrypt/decrypt
  uint16_t len;           ///< length of data u
  uint32_t reserved;      ///< for future use
  int32_t errorOut;       ///< error code
};

//-----------------------------------
// Notifications - APDU Data
//-----------------------------------
typedef enum QBConaxAPDUSource_
{
  QBConaxAPDUSource_ECM    = 0,
  QBConaxAPDUSource_EMM    = 1,
  QBConaxAPDUSource_Other  = 2,
  QBConaxAPDUSource_MAX    = 3
} QBConaxAPDUSource;

#define QBConaxAPDUDataMaxLen  (256+8)

/// Data notified with @ref QBConaxNotification_APDU_data
/// Module should send each unrecognised APDU to application
/// PROTECTED_DATA should be first decrypted and all APDUs from it should be sent with this notification
struct QBConaxNotificationAPDUData {
  uint8_t source;                         ///< QBConaxAPDUSource
  uint32_t len;                           ///< length of valid data in @a data buffer
  uint8_t data[QBConaxAPDUDataMaxLen];    ///< data containing APDU_ID,APDU_L and APDU_V
};


//-----------------------------------
// Notifications - URI Data
//-----------------------------------
//max allowed in BRIDGE_CONTROL_APDU
#define QBConaxBridgeControlApduId (0x65)
#define QBConaxUriDataMaxLen  (256)

/// Data notified with @ref QBConaxNotification_uri_data
/// Module should send each URI received from HOST_DATA inside PROTECTED_DATA, URI received without LINK PROTECTION should be dropped
struct QBConaxNotificationUriData {
  uint8_t apdu_id;                    ///< should be always 0x65, just to validate
  uint8_t len;                        ///< length of valid data in @a data buffer
  uint8_t flags;                      ///< not used now, should be set to ZERO for forward compatibility
  uint8_t data[QBConaxUriDataMaxLen]; ///< value from bridge_control() i.e. APDU_V
};


//-----------------------------------
// Notifications - all
//-----------------------------------

/** Output parameter when querying the module about recent results.
 *  @see QBConaxGetNotification
 */
struct QBConaxCmdNotification
{
  /// [out] type of the notification, @ref QBConaxNotification
  uint8_t  type;

  /// [out] union of all data types that can come with a notification
  union QBConaxNotificationUnion
  {
    uint32_t  val;  ///< when @a type is @ref QBConaxNotification_no_access or @ref QBConaxNotification_user_text

    struct QBConaxNotificationCorrectCard     correctCard;    ///< when @a type is @ref QBConaxNotification_card_state_correct
    struct QBConaxNotificationMaturityRating  maturityRating; ///< when @a type is @ref QBConaxNotification_maturity_rating
    struct QBConaxNotificationTokenDebit      tokenDebit;     ///< when @a type is @ref QBConaxNotification_token_debit
    struct QBConaxNotificationAcceptViewing   acceptViewing;  ///< when @a type is @ref QBConaxNotification_accept_viewing
    struct QBConaxNotificationOrder           order;          ///< when @a type is @ref QBConaxNotification_order

    struct QBConaxNotificationPairingChanged  pairingChanged; ///< when @a type is @ref QBConaxNotification_pairing_changed
    struct QBConaxNotificationCustomData      customData;     ///< when @a type is @ref QBConaxNotification_custom_data
    struct QBConaxNotificationAPDUData        apduData;       ///< when @a type is @ref QBConaxNotification_apdu_data
    struct QBConaxNotificationUriData         uriData;        ///< when @a type is @ref QBConaxNotification_uri_data
    struct QBConaxCmdEncrypt                  encrypt;        ///< when @a type is @ref QBConaxNotification_encrypt

    QBDescramblerCW  controlWord;                             ///< when @a type is @ref QBConaxNotification_control_word
  } u;

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};


//-----------------------------------
// Helpers
//-----------------------------------

// Helper struct, not part of the official api.
struct QBConaxStructUnion
{
  union {
    struct QBConaxCmdHostInfo  host_info;
    struct QBConaxCmdCardInfo  card_info;

    struct QBConaxCmdSubscriptionInfo  subs;
    struct QBConaxCmdEventInfo   event;
    struct QBConaxCmdPurseInfo   purse;
    struct QBConaxCmdDebitInfo   debit;
    struct QBConaxCmdCreditInfo  credit;

    struct QBConaxCmdUserText     ut;
    struct QBConaxCmdFingerprint  fp;

    //struct QBConaxCmdPushECM    ecm;

    struct QBConaxCmdParseCAT cat;
    //struct QBConaxCmdPushEMM  emm;

    struct QBConaxCmdNotification  notification;

    struct QBConaxCmdOptions  options;

    struct QBConaxCmdEncrypt encrypt;
  } u;
};

#endif // #ifndef QB_CONAX_STRUCTURES_H

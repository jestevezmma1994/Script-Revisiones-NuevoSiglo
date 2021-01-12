/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CONAX_INTERNAL_IOCTL_H
#define QB_CONAX_INTERNAL_IOCTL_H

#include <QBConaxInternal/structures.h>

#ifndef MODULE
#include <sys/ioctl.h>
#endif

#if ! defined QB_CONAX_IOCTL_GROUP_NUMBER
#  define QB_CONAX_IOCTL_GROUP_NUMBER  230
#endif

//------------------------------
// init
//------------------------------

#define QB_CONAX_IOCTL_INIT               _IOR( QB_CONAX_IOCTL_GROUP_NUMBER,  0, struct QBConaxCmd*)
#define QB_CONAX_IOCTL_DEINIT             _IOR( QB_CONAX_IOCTL_GROUP_NUMBER,  1, struct QBConaxCmd*)

//------------------------------
// getting info
//------------------------------

#define QB_CONAX_IOCTL_GET_HOST_INFO      _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER,  2, struct QBConaxCmdHostInfo*)
#define QB_CONAX_IOCTL_GET_CARD_INFO      _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER,  3, struct QBConaxCmdCardInfo*)

#define QB_CONAX_IOCTL_GET_SUBS_COUNT     _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER,  4, struct QBConaxCmdCount*)
#define QB_CONAX_IOCTL_GET_SUBS_INFO      _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER,  5, struct QBConaxCmdSubscriptionInfo*)

#define QB_CONAX_IOCTL_GET_EVENT_COUNT    _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER,  6, struct QBConaxCmdCount*)
#define QB_CONAX_IOCTL_GET_EVENT_INFO     _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER,  7, struct QBConaxCmdEventInfo*)

#define QB_CONAX_IOCTL_GET_PURSE_COUNT    _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER,  8, struct QBConaxCmdCount*)
#define QB_CONAX_IOCTL_GET_PURSE_INFO     _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER,  9, struct QBConaxCmdPurseInfo*)

#define QB_CONAX_IOCTL_GET_DEBIT_COUNT    _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 10, struct QBConaxCmdCount*)
#define QB_CONAX_IOCTL_GET_DEBIT_INFO     _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 11, struct QBConaxCmdDebitInfo*)

#define QB_CONAX_IOCTL_GET_CREDIT_COUNT   _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 12, struct QBConaxCmdCount*)
#define QB_CONAX_IOCTL_GET_CREDIT_INFO    _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 13, struct QBConaxCmdCreditInfo*)

//------------------------------
// pin
//------------------------------

#define QB_CONAX_IOCTL_ENTER_PIN          _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 14, struct QBConaxCmdEnterPIN*)
#define QB_CONAX_IOCTL_CHANGE_PIN         _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 15, struct QBConaxCmdChangePIN*)

#define QB_CONAX_IOCTL_CHANGE_MAT_RATING  _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 16, struct QBConaxCmdChangeMaturityRating*)

//------------------------------
// user text
//------------------------------

#define QB_CONAX_IOCTL_GET_USER_TEXT      _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 17, struct QBConaxCmdUserText*)
#define QB_CONAX_IOCTL_MARK_USER_TEXT     _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 18, struct QBConaxCmd*)

#define QB_CONAX_IOCTL_GET_FINGERPRINT    _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 19, struct QBConaxCmdFingerprint*)
#define QB_CONAX_IOCTL_MARK_FINGERPRINT   _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 20, struct QBConaxCmdFingerprint*)

//------------------------------
// descrambler
//------------------------------

#define QB_CONAX_IOCTL_UPDATE_SCRAMBLED_STATUS  _IOR( QB_CONAX_IOCTL_GROUP_NUMBER, 21, struct QBConaxCmdUpdateScrambledStatus*)

#define QB_CONAX_IOCTL_UPDATE_SCRAMBLING_MODE   _IOR( QB_CONAX_IOCTL_GROUP_NUMBER, 22, struct QBConaxCmdUpdateScramblingMode*)

#define QB_CONAX_IOCTL_PUSH_ECM           _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 23, struct QBConaxCmdPushECM*)

//------------------------------
// cat/emm
//------------------------------

#define QB_CONAX_IOCTL_PARSE_CAT          _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 24, struct QBConaxCmdParseCAT*)
#define QB_CONAX_IOCTL_PUSH_EMM           _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 25, struct QBConaxCmdPushEMM*)

//------------------------------
// notifications
//------------------------------

#define QB_CONAX_IOCTL_GET_NOTIFICATION   _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 26, struct QBConaxCmdNotification*)


//------------------------------
// misc
//------------------------------

#define QB_CONAX_IOCTL_STANDBY            _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 27, struct QBConaxCmdStandby*)

#define QB_CONAX_IOCTL_OPTIONS            _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 28, struct QBConaxCmdOptions*)

#define QB_CONAX_IOCTL_RESET_SESSION      _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 29, struct QBConaxCmdResetSession*)

#define QB_CONAX_IOCTL_ENCRYPT            _IOWR(QB_CONAX_IOCTL_GROUP_NUMBER, 30, struct QBConaxCmdEncrypt*)


#endif // #ifndef QB_CONAX_INTERNAL_IOCTL_H

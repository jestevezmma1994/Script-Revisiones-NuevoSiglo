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

#ifndef QB_CONAX_INTERNAL_STRUCTURES_H
#define QB_CONAX_INTERNAL_STRUCTURES_H

#include "QBConax/structures.h"

/**
 * @file QBConaxInternal/structures.h
 * @brief Data types used by QBConax internal api.
 **/

/**
 * @defgroup QBConaxInternal QBConax internal structures
 * @{
 **/

//------------------------------
// misc
//------------------------------

/// Used in ioctl layer only
struct QBConaxCmd
{
  /// [in]
  uint32_t  id;

  /// [out], must be used only in ioctl layer
  int32_t  errorOut;
};

/// Used in ioctl layer only
struct QBConaxCmdCount
{
  /// [in]
  uint32_t  id;

  /// [out]
  int32_t  cnt;

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};


//------------------------------
// PIN
//------------------------------

/// Used in ioctl layer only
struct QBConaxCmdEnterPIN
{
  /// [in]
  uint8_t   sessionId; // session id that we want to unblock (unused for generic pin entry)
  enum QBConaxPINType  type;
  uint8_t  code[4];

  // [out] QBConaxPINStatus_*, or negative for generic error, must be used only in ioctl layer
  int32_t  errorOut;
};

/// Used in ioctl layer only
struct QBConaxCmdChangePIN
{
  /// [in]
  uint8_t  codeOld[4];
  uint8_t  codeNew[4];

  // [out] QBConaxPINStatus_*, or negative for generic error, must be used only in ioctl layer
  int32_t  errorOut;
};

/// Used in ioctl layer only
struct QBConaxCmdChangeMaturityRating
{
  /// [in]
  uint8_t  code[4];
  uint8_t  newRating;

  /// [out] QBConaxPINStatus_*, or negative for generic error, must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// descrambling
//------------------------------

/// Used in ioctl layer only
struct QBConaxCmdUpdateScrambledStatus
{
  // [in]
  uint8_t  sessionId;   ///< sessionId, as in Conax specs
  uint8_t  isScrambled; ///< 0 for FTA, 1 for scrambled

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

/// Used in ioctl layer only
struct QBConaxCmdUpdateScramblingMode
{
  // [in]
  uint8_t  sessionId; ///< sessionId, as in Conax specs
  uint8_t  mode;      ///< as found in the PMT (@c 0 iff unknown)

  // [out], must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// CAT/EMM
//------------------------------

/// Used in ioctl layer only
struct QBConaxCmdPushEMM
{
  /// [in]
  int32_t pid; /// -1 iff not DVB, else valid TS pid
  uint32_t len;
  uint8_t  section[256];

  /// [out], must be used only in ioctl layer
  int32_t  errorOut;
};


//------------------------------
// Standby
//------------------------------

/// Used in ioctl layer only
struct QBConaxCmdStandby
{
  /// [in]
  uint8_t  standbyState; /// @see QBConaxStandbyState

  /// [out], must be used only in ioctl layer
  int32_t  errorOut;
};


//------------------------------
// Misc
//------------------------------

struct QBConaxCmdArg
{
  /// [in]
  uint32_t  opt;
  void*     prv;

  /// [out], must be used only in ioctl layer
  int32_t  errorOut;
};

//------------------------------
// Reset Session
//------------------------------

/// Used in ioctl layer only
struct QBConaxCmdResetSession
{
  /// [in]
  uint8_t  session_id; /// @see conax specification

  /// [out], must be used only in ioctl layer
  int32_t  errorOut;
};

/**
 * @}
 **/

#endif // #ifndef QB_CONAX_INTERNAL_STRUCTURES_H

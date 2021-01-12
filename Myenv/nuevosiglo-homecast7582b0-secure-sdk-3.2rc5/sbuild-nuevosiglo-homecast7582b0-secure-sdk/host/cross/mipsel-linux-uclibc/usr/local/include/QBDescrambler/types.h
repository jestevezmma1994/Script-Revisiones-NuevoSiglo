/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DESCRAMBLER_TYPES_H
#define QB_DESCRAMBLER_TYPES_H

/**
 * @file SMP/HAL/QBDescrambler/proto/types.h
 * @brief Data types used by descrambler API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup QBDescramblerTypes
 * @{
 **/

typedef struct QBDescramblerCADescriptor_s QBDescramblerCADescriptor;
struct QBDescramblerCADescriptor_s {
    uint16_t ecmPid;  ///< on which pid ecms are
    uint16_t systemId;  ///< for which system id these ecms are for
    int16_t esPid;   ///< -1 if all pids are scrambled with this ecm
};

/// Actual channel type is platform-dependent.
/// This struct declaration is only here for increased type-safety.
typedef struct QBDescramblerPidChannelStub_s QBDescramblerPidChannelStub;

/// Command for applying given control-words to descramblers, according to CA configuration, and ecm pid.
struct QBDescramblerUpdateCWCmd_s {
    uint16_t ecmPid;   ///< from which ecm pid the cw originated

    uint16_t systemId; ///< of the card that was used to descramble this cw (always current card if in online mode)
    uint8_t isOnline;  ///< cw was received from current card (live), or from pvr (offline, current card is not used at all)

    QBDescramblerCW cws;    ///< the control words that should used as the update
};
typedef struct QBDescramblerUpdateCWCmd_s QBDescramblerUpdateCWCmd;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DESCRAMBLER_TYPES_H

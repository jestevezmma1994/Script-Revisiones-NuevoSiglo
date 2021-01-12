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

#ifndef QB_DESCRAMBLER_ENGINE_H
#define QB_DESCRAMBLER_ENGINE_H

/**
 * @file SMP/HAL/QBDescrambler/proto/engine.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief Descrambler engine API
 **/

#include "types.h"

#ifdef __cplusplus__
extern "C" {
#endif


/**
 * @defgroup QBDescramblerEngine Descrambler engine
 * @ingroup QBDescrambler
 * @{
 **/

/*
 * QBDescramblerEngine API
 *
 * Some of pidChannels could be protected by Conditional Access Systems.
 * When content is protected data in pidChannels is scrambled.
 *
 * Using this API it is possible to give to hardware descramblers information
 * which pidChannles are scrambled and provide keys. After this operation data
 * read from pidChannels is descrmabled.
 *
 * It is also possible to scramble content using this engine.
 */


// TODO: Change contextId to SvObject

/**
 * Initialize all descramblers
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineInit(void);
/**
 * Destroy descramblers
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineDeinit(void);

/**
 * Create descrambler engine resource
 * @return context id or @c -1 in case of error or lack of resources
 */
int QBDescramblerEngineCreateContext(void);

/**
 * Get free descrambler engine resource
 * @param[in] contextId handle for descrambler engine context
 * @return @c 0 on success, @c -1 in case of error
 */
void QBDescramblerEngineReleaseContext(const int contextId);

/**
 * Update information about ecmPid <-> esPid association
 * @param[in] contextId descrambler session id
 * @param[in] tab array with information about connection between ecm and es PID
 * @param[in] cnt number of entries in tab
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineUpdateCA(int contextId, const QBDescramblerCADescriptor* tab, int cnt);

/**
 * Add pid channal to descrambler
 * @param[in] contextId descrambler session id
 * @param[in] channel HAL handle of channel
 * @param[in] pid channel pid
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineAddChannel(int contextId, QBDescramblerPidChannelStub* channel, uint16_t pid);

/**
 * Remove pid channal from descrambler
 * @param[in] contextId descrambler session id
 * @param[in] channel HAL handle of channel
 * @param[in] pid channel pid
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineRemoveChannel(int contextId, QBDescramblerPidChannelStub* channel, uint16_t pid);

/**
 * Update keys in descrambler
 * @param[in] contextId descrambler session id
 * @param[in] cmd information about keys: ecmPid control words, protection mode, used algorithm
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineUpdateCW(int contextId, const QBDescramblerUpdateCWCmd* cmd);

/**
 * Clear descrambling keys
 * @param[in] contextId descrambler session id
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineClearCWs(int contextId);

/**
 * Check if descrambler supports AES with given block termination mode.
 * @param[in] mode  block termination mode we want to use
 * @return 1 iff supported, 0 iff not supported, nagative on error
 */
int QBDescramblerEngineSupportsTermMode(QBDescramblerTermMode mode);


/**
 * Set algorithm that should be used to protect content
 * @param[in] contextId descrambler session id
 * @param[in] algo algorithm
 * @param[in] protection how control words are protected
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineSetScramblingAlgorithm(int contextId, QBDescramblerAlgo algo, QBDescramblerCWProtectionMode protection);

/**
 * Set keys in scrambler and enabled/disable scrambling
 * @param[in] contextId descrambler session id
 * @param[in] ecmPid ecm pid value used to select related channel, in case of @c -1 all channels are updated with provided keys.
 * @param[in] keys control words, control word protection key, protection mode, used algorithm
 * @param[in] mode none means no scrambling, even and odd is polarity which will be set on ts packets after scrambling
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineSetScramblingKeys(int contextId, int16_t ecmPid, const QBDescramblerCW *keys, QBDescramblerScramblingMode mode);

/**
 * Invalidate scrambling keys and algorithm
 *
 * @param[in] contextId descrambler session id
 * @return 0 on success, error_code < 0 on error
 */
int QBDescramblerEngineInvalidateScrambling(int contextId);

/**
 * @}
 **/


#ifdef __cplusplus__
}
#endif

#endif // #ifndef QB_DESCRAMBLER_ENGINE_H

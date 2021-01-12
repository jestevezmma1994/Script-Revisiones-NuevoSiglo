/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_IP_STREAM_BITRATE_MANAGER_H
#define QB_IP_STREAM_BITRATE_MANAGER_H

/**
 * @file  QBIPStreamBitrateManager.h
 * @brief Streaming bitrate manager. Used to calculate real download
 * bitrate which comes through streaming.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvObject.h>

/**
 * @defgroup QBIPStreamBitrateManager QBIPStreamBitrateManager: Streaming bitrate manager
 * @{
 **/

/**
 * Player buffer duration below which challenge mode is cancelled
 **/
#define PANIC_BUFFER_DURATION_SEC (1.0)

/**
 * Streaming bitrate manager used to calculate real download bitrate.
 *
 * @class QBIPStreamBitrateManager
 * @extends SvObject
 **/
typedef struct QBIPStreamBitrateManager_ *QBIPStreamBitrateManager;

/**
 * Create IP stream bitrate manager.
 *
 * @memberof QBIPStreamBitrateManager
 *
 * @param[in] historyDepth      number of chunks's bitrates included in history
 * @param[in] variantsCount     number of variants to keep history of.
 * @return                      new created IP stream bitrate manager,
 *                              otherwise @c NULL in case of error
 **/
QBIPStreamBitrateManager
QBIPStreamBitrateManagerCreate(int historyDepth, int variantsCount);

/**
 * Get current bitrate (bits per second) which is calculated from incoming bytes and history.
 *
 * Current variant is set by QBIPStreamBitrateManagerSetVariant method.
 *
 * @memberof QBIPStreamBitrateManager
 *
 * @param[in] self              streaming bitrate manager handle
 * @return                      current weighted bitrate (bits per second)
 **/
int
QBIPStreamBitrateManagerGetDownloadBitrate(QBIPStreamBitrateManager self);

/**
 * Get download success ratio for given variant.
 *
 * Successful download is defined as download with bitrate higher than required including history
 *
 * @param[in] self              streaming bitrate manager handle
 * @param[in] variant           variant index
 * @return                      success ratio as a percentage (0-100)
 */
int
QBIPStreamBitrateManagerGetSuccessRatio(QBIPStreamBitrateManager self, int variant);

/**
 * Set current variant.
 *
 * This method selects which history should be used.
 *
 * @param[in] self              streaming bitrate manager handle
 * @param[in] variant           variant index
 * @param[in] bitrate           variant bitrate
 */
void
QBIPStreamBitrateManagerSetVariant(QBIPStreamBitrateManager self, int variant, int bitrate);

/**
 * Set number of variants to keep history for.
 *
 * @param[in] self              streaming bitrate manager handle
 * @param[in] variantsCount     number of variants to keep history of.
 */
void
QBIPStreamBitrateManagerSetVariantsCount(QBIPStreamBitrateManager self, int variantsCount);

/**
 * Set playback info provider.
 *
 * Provider is an implementation of QBIPStreamPlaybackInfoProviderIface.
 *
 * @param[in] self                 streaming bitrate manager handle
 * @param[in] playbackInfoProvider playback info provider handle
 */
void
QBIPStreamBitrateManagerSetPlaybackInfoProvider(QBIPStreamBitrateManager self, SvObject playbackInfoProvider);

/**
 * Set or unset challenge mode.
 *
 * Challenge mode is a situation when
 * - current bitrate is too low to keep selected variant, but buffer is long enough to not decrease quality
 * - higher quality is selected eventhough current bitrate is too low (server may increase bandwith)
 *
 * @param[in] self              streaming bitrate manager handle
 * @param[in] enabled           challenge mode flag
 */
void
QBIPStreamBitrateManagerSetChallengeMode(QBIPStreamBitrateManager self, bool enabled);

/**
 * Get challenge mode status.
 *
 * Challenge mode is a situation when
 * - current bitrate is too low to keep selected variant, but buffer is long enough to not decrease quality
 * - higher quality is selected eventhough current bitrate is too low (server may increase bandwith)
 *
 * @param[in] self              streaming bitrate manager handle
 * @return                      challenge mode flag
 */
bool
QBIPStreamBitrateManagerGetChallengeMode(QBIPStreamBitrateManager self);

/**
 * Get challenge mode result.
 *
 * Challenge mode is successful when chunk was downloaded completely,
 * even if bitrate was lower than required.
 * In case of failure downloader emits rollback status.
 *
 * @param[in] self              streaming bitrate manager handle
 * @return                      challenge mode result
 */
bool
QBIPStreamBitrateManagerGetChallengeModeResult(QBIPStreamBitrateManager self);

/**
 * @}
 **/

#endif /* QB_IP_STREAM_BITRATE_MANAGER_H */

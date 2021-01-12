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

#ifndef QB_IP_STREAM_PLAYBACK_INFO_PROVIDER_IFACE_H_
#define QB_IP_STREAM_PLAYBACK_INFO_PROVIDER_IFACE_H_

/**
 * @file  QBIPStreamPlaybackInfoProviderIface.h Playback information provider interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/Common/QBIPStreamManagerTypes.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBIPStreamPlaybackInfoProviderIface Playback information provider interface.
 * @{
 **/

/**
 * Playback position and buffered time of the player.
 **/
typedef struct QBIPStreamPlaybackPositions_ {
    double position;        //!< playback position in seconds
    uint64_t playedPTS;     //!< playback position in 90k
    uint64_t bufferedPTS;   //!< buffered time in 90k
} *QBIPStreamPlaybackPositions;

/**
 * Playback information provider interface.
 * @interface QBIPStreamBitrateSelectorIface
 */
typedef struct QBIPStreamPlaybackInfoProviderIface_ *QBIPStreamPlaybackInfoProviderIface;

struct QBIPStreamPlaybackInfoProviderIface_ {
    /**
     * Get player positions.
     *
     * @param[in] self                      interface object
     * @param[out] outPositions             position data
     * @return                              player position in seconds or @c -1 when unknown
     */
    void (*getPlayerPositions)(SvObject self, QBIPStreamPlaybackPositions outPositions);

    /**
     * Get next chunk duration.
     *
     * @param[in] self                      interface object
     * @return                              get next chunk duration or @c -1 when unknown
     */
    double (*getNextChunkDuration)(SvObject self);

    /**
     * Get next chunk position (PTS).
     *
     * @param[in] self                      interface object
     * @return                              get next chunk position or @c -1 when unknown
     */
    double (*getNextChunkPosition)(SvObject self);
};

/**
 * Get runtime type identification object representing
 * QBIPStreamPlaybackInfoProviderIface interface.
 *
 * @return representing interface object
 **/
SvInterface
QBIPStreamPlaybackInfoProviderIface_getInterface(void);

/**
 * @}
 */

#endif /* QB_IP_STREAM_PLAYBACK_INFO_PROVIDER_IFACE_H_ */

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCASPVRSETTINGS_H_
#define QBCASPVRSETTINGS_H_

/**
 * @file QBCASPVRSettings.h QBCASPVRSettings
 */

#include <mpeg_ca_generator.h>
#include <stdbool.h>

/**
 * PVR customization settings for implemented CAS
 */
struct QBCASPVRSettings_s {
    bool shouldRemoveEcmFromPVR; /**< Check CAS requirements about removing CAS ECM from HDD (e.g. Latens CAS require it) */
    bool shouldHaveStandalonePVRSession; /**< @c true is PVR session should not be connected to playback session (e.g. Conax Enhanced PVR use it) */
    bool shouldNotReencryptPlaybackSession; /**< @c false if playback session can be rescrambled */
    mpeg_ca_generator_new_ca_table_mode newCaTableMode; /**< new ca table generation mode */
    bool isIpReencryptionSupported; /** @c true if reencryption for IP content is supported. */
    bool shouldUseBlockEncryption; /** @c true if block encryption mode should be used to reencrypt content */
};

#endif /* QBCASPVRSETTINGS_H_ */

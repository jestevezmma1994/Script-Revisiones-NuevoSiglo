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


#ifndef QBVOLUME_H
#define QBVOLUME_H

#include <SvFoundation/SvCoreTypes.h>
#include <stdbool.h>


#define USER_NAME_IN_VOLUME_MUTE_TABLE "user"


typedef struct QBVolume_ *QBVolume;

/**
 * Create QBVolume service.
 *
 * @return                      created volume service instance, @c NULL if failed
 **/
QBVolume QBVolumeCreate(void);

/**
 * Get runtime type identification object representing QBVolume class.
 *
 * @return QBVolume type identification object
 **/
SvType QBVolume_getType(void);

void QBVolumeMute(QBVolume self, const SvString muteName, bool mute);

void QBVolumeUp(QBVolume self);

void QBVolumeDown(QBVolume self);

/**
 * Set fixed volume level.
 *
 * @param self      handle to QBVolume object
 * @param volume    volume level to be set
 */
void QBVolumeSet(QBVolume self, unsigned int volume);

/**
 * Get current volume level.
 *
 * @param self      handle to QBVolume object
 * @return          current volume level in range <0:QBVIEWPORT_VOL_MAX>
 */
unsigned int QBVolumeGetCurrentLevel(QBVolume self);

/**
 * Lock volume at fixed level. This is runtime setting which isn't stored
 * (only new volume level is remembered).
 *
 * @param self      handle to QBVolume object
 * @param volume    volume level to be locked (negative value to lock current level)
 */
void QBVolumeLock(QBVolume self, int volume);

/**
 * Unlock volume level.
 *
 * @param self      handle to QBVolume object
 */
void QBVolumeUnlock(QBVolume self);

/**
 * Get current volume level, mute state and lock state.
 *
 * @param[in] self          handle to QBVolume object
 * @param[out] level        current volume level
 * @param[out] isMuted      current mute state
 * @param[out] isLocked     current QBVolume lock state
 */
void QBVolumeGetCurrentState(QBVolume self, int *level, bool *isMuted, bool *isLocked);

void QBVolumeRestoreToDefaults(QBVolume self);

/*
 * @brief Restore volume to user defined settings (reads it from a file)
 * @param self   handle to QBVolume object
 */
void QBVolumeRestoreToUserSettings(QBVolume self);

/**
* @brief Check if volume is muted.
* @param[in] self       handle to QBVolume object
* @param[out] errorOut  error info
* @return               @c true if volume is muted otherwise @c false
**/
bool QBVolumeIsMuted(QBVolume self, SvErrorInfo *errorOut);

/**
* @brief Check if volume is muted by user.
* @param[in] self       handle to QBVolume object
* @param[out] errorOut  error info
* @return               @c true if volume is muted by user, @c false otherwise
**/
bool QBVolumeIsMutedByUser(QBVolume self, SvErrorInfo *errorOut);

/**
* @brief Check if volume control is locked.
* @param[in] self       handle to QBVolume object
* @param[out] errorOut  error info
* @return               @c true if volume is locked, @c false otherwise
**/
bool QBVolumeIsLocked(QBVolume self, SvErrorInfo *errorOut);

#endif /* QBVOLUME_H */

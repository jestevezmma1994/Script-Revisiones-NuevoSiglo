/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DISK_PVR_STORAGE_MODE_H_
#define QB_DISK_PVR_STORAGE_MODE_H_

/**
 * @file QBDiskPvrStorageMode.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief Disk pvr storage helper
 **/

/**
 * @defgroup QBDiskPvrStorageMode Disk PVR storage mode helper
 * @ingroup CubiTV_configurations
 *
 * CA vendors may have some restriction about how PVR or timeshifted content should be stored on HDD.
 * Two major modes can be indentified
 *  - retention - store content as recived.
 *  - reencryption - content shall be reencrypted before storing on HDD
 *
 * Usually each STB model is certifed with only one mode, so it should be chosen wisely.
 * Recordings acquired in one mode are not compatible with different ones. If mode is changed at STB,
 * old recordings are likely to be removed, or they will not be usable.
 *
 * They are two ways how storage mode can be defined:
 * - conf.json at 'PVR_REENCRYPTION' key - this method is deprecated and should not be used in new projects
 * - project file env CUBITV_PVR_REENCRYPTION - preferred method, it has priorty above conf.json
 *
 * Supported values:
 * - disabled - retention mode will be used
 * - enabled - reencryption mode will be used
 * - dvbonly - reencryption will be used only for streams acquired from DVB
 *
 * If mode is not setup, default mode will be returned which is retention
 *
 * @{
 **/

/**
 * @brief Disk pvr storage mode indicator.
 */
typedef enum QBDiskPvrStorageMode_e {
    QBDiskPvrStorageMode_retention,             /**< PVR and timeshifted content should be stored as recived */
    QBDiskPvrStorageMode_reencryption,          /**< PVR and timeshifted content shall be stored reencrypted */
    QBDiskPvrStorageMode_dvbOnlyReencryption,   /**< PVR and timeshifted content shall be stored reencrypted only if is aquired from dvb */
} QBDiskPvrStorageMode;

/**
 * @brief Get pvr storage mode which shall be used by application
 * @return disk pvr storage mode
 */
QBDiskPvrStorageMode QBDiskPvrStorageModeGet(void);

/**
 * @}
 **/

#endif // QB_DISK_PVR_STORAGE_MODE_H_


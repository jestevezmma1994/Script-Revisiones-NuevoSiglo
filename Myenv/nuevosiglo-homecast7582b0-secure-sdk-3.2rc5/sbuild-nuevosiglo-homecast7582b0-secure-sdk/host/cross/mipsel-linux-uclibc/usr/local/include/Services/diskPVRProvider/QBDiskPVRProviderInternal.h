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

#ifndef QB_DISK_PVR_PROVIDER_INTERNAL_H_
#define QB_DISK_PVR_PROVIDER_INTERNAL_H_

/**
 * @file QBDiskPVRProviderInternal.h Disk PVR provider internal methods header.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBDiskPVRProvider.h"
#include <SvEPGDataLayer/Data/SvTVChannel.h>

/**
 * @addtogroup QBDiskPVRProvider
 * @{
 **/

/**
 * Disk PVR recordings priorities.
 */
typedef enum {
    QBDiskPVRPriority_keyword = 10,     ///< keyword recording priority
    QBDiskPVRPriority_repeat = 15,      ///< repeated recording priority
    QBDiskPVRPriority_event = 20,       ///< event recording priority
    QBDiskPVRPriority_OTR = 30,         ///< one time recording recording priority
    QBDiskPVRPriority_manual = 40,      ///< manual recording priority
} QBDiskPVRPriority;

/**
 * Get PVR cost class from given channel.
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] channel           channel
 * @return                      cost class of a channel
 **/
QBDiskPVRProviderChannelCostClass
QBDiskPVRProviderGetPVRCostClassFromChannel(SvTVChannel channel);

/**
 * Notify listeners about new directory created.
 *
 * @param[in] self      PVR provider handle
 * @param[in] dir       PVR directory handle
 **/
SvHidden void
QBDiskPVRProviderNotifyDirectoryAdded(QBDiskPVRProvider self, QBPVRDirectory dir);

/**
 * @}
 **/

#endif /* QB_DISK_PVR_PROVIDER_INTERNAL_H_ */

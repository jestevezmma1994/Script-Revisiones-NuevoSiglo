/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBPVRMOUNTER_H_
#define QBPVRMOUNTER_H_

#include <stdbool.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <Services/core/hotplugMounts.h>
#include <main_decl.h>

typedef enum
{
    QBPVRMounterState_no_state = 0,
    QBPVRMounterState_disk_checking,
    QBPVRMounterState_disk_mounted,
    QBPVRMounterState_disk_compatible,
    QBPVRMounterState_disk_formatting,
    QBPVRMounterState_disk_mounting,
    QBPVRMounterState_disk_unmounting,
    QBPVRMounterState_error_formatting,
    QBPVRMounterState_error_mounting,
    QBPVRMounterState_error_unmounting,
    QBPVRMounterState_disk_incompatible,

} QBPVRMounterState;

struct QBDiskStatus_t {
    struct SvObject_ super_;
    QBDiskInfo disk;
    QBPVRMounterState state;
    bool pvr;
    bool ts;
    int recLimit; // value -1 means limit is not set / undefined
    SvArray otherPartitions;
};
typedef struct QBDiskStatus_t* QBDiskStatus;


typedef struct QBPVRMounter_t* QBPVRMounter;

struct QBPVRMounterListener_t {
    void (*mounterResultReceived)(SvObject self_, QBDiskStatus disk);
};
typedef struct QBPVRMounterListener_t* QBPVRMounterListener;

SvInterface QBPVRMounterListener_getInterface(void);

/**
 * Get runtime specification of QBPVRMounter type
 *
 * @return QBPVRMounter runtime type specification
 */
SvType QBPVRMounter_getType(void);

QBPVRMounter QBPVRMounterCreate(SvScheduler scheduler, SvString possiblePartitionsPath);
void QBPVRMounterAddListener(QBPVRMounter self, SvObject listener);
void QBPVRMounterRemoveListener(QBPVRMounter self, SvObject listener);

/**
 * Attempts to mount device with given ID. Result of operation will be returned through QBPVRMounterListener.
 *
 * @param[in] self  QBPVRMounter instance
 * @param[in] id    device id to mount
 */
void QBPVRMounterMount(QBPVRMounter self, SvString id);

/**
 * Attempts to unmount device with given ID. Result of operation will be returned through QBPVRMounterListener.
 *
 * @param[in] self  QBPVRMounter instance
 * @param[in] id    device id to unmount
 */
void QBPVRMounterUnmount(QBPVRMounter self, SvString id);
void QBPVRMounterFormat(QBPVRMounter self, SvString shortNode, SvString formatPartition);
void QBPVRMounterStart(QBPVRMounter self, AppGlobals appGlobals);
bool QBPVRMounterIsPVRUsed(QBPVRMounter self, SvString id);
bool QBPVRMounterIsFormatted(QBPVRMounter self, SvString id);
QBPVRMounterState QBPVRMounterGetState(QBPVRMounter self, SvString id);
bool QBPVRMounterIsBusy(QBPVRMounter self);
QBDiskStatus QBPVRMounterGetDiskStatus(QBPVRMounter self, SvString id);
void QBPVRMounterAllowPVR(QBPVRMounter self, bool allowPVR);
void QBPVRMounterAllowTS(QBPVRMounter self, bool allowTS);

/**
 * Turns on/off mounting of PVR file system (QBRecordFS). To take effect must be called before any partition mounting.
 * This function is useful when you want to access disk directly omitting QBRecordFS, cause writing simultaneously to
 * disk when PVR file system is mounted can lead to crash of QBRecordFS.
 *
 * @param[in] self          QBPVRMounter instance
 * @param[in] enabled       turn on/off mounting
 */
void QBPVRMounterEnablePVRFileSystemMounting(QBPVRMounter self, bool enabled);

/**
 * Get path to root of meta partition
 *
 * @param[in] self QBPVRMounter instance
 * @return SvString with path to root of meta partition
 */
SvString QBPVRMounterGetMetaRoot(QBPVRMounter self);

/**
 * Get path to data device
 *
 * @param[in] self QBPVRMounter instance
 * @return SvString with path to data device
 */
SvString QBPVRMounterGetDataDev(QBPVRMounter self);

#endif


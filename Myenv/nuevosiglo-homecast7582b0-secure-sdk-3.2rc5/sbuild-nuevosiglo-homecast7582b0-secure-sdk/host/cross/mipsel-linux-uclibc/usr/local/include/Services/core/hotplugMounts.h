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

#ifndef HOTPLUG_MOUNTS_H_
#define HOTPLUG_MOUNTS_H_

#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvInterface.h>
#include <QBDataModel3/QBActiveTree.h>

struct QBDiskInfo_t {
    struct SvObject_ super_;
    SvString vendor;
    SvString model;
    SvString devId;
    SvString devNode;
    SvString devSerial;
    long long int size;
    int partCount;
    bool isInternal;
    SvArray partitions;
};
typedef struct QBDiskInfo_t* QBDiskInfo;

struct QBPartitionInfo_t {
    struct SvObject_ super_;
    SvString partId;
    SvString devNode;
    SvString rootNode;
    SvString type;
    SvString label;
    long long int size;
};
typedef struct QBPartitionInfo_t* QBPartitionInfo;

typedef struct QBHotplugMountAgent_t *QBHotplugMountAgent;

struct QBHotplugMountAgentListener_t {
    void (*partitionAdded)(SvObject self_, SvObject path, QBDiskInfo disk, QBPartitionInfo part);
    void (*partitionRemoved)(SvObject self_, SvObject path, SvString remId, SvString diskId);
    void (*diskAdded)(SvObject self_, SvObject path, QBDiskInfo disk);
    void (*diskRemoved)(SvObject self_, SvObject path, SvString remId, SvString diskId);
};
typedef struct QBHotplugMountAgentListener_t* QBHotplugMountAgentListener;

extern SvInterface
QBHotplugMountAgentListener_getInterface(void);

extern void
QBHotplugMountAgentAddListener(QBHotplugMountAgent self, SvObject listener);

extern void
QBHotplugMountAgentRemoveListener(QBHotplugMountAgent self, SvObject listener);

/**
 * Create QBHotplugMountAgent.
 *
 * @param[in] scheduler         scheduler
 * @return                      created hotplug mount agent instance, @c NULL if failed
 **/
extern QBHotplugMountAgent
QBHotplugMountAgentCreate(SvScheduler scheduler);

extern void
QBHotplugMountAgentStart(QBHotplugMountAgent agent, QBActiveTree menuTree);

extern void
QBHotplugMountAgentStop(QBHotplugMountAgent agent);

extern SvArray
QBHotplugMountAgentGetAvailableDisks(QBHotplugMountAgent self);

#endif // #ifndef HOTPLUG_MOUNTS_H_


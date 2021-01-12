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

#ifndef QB_UPGRADE_MONITOR_H_
#define QB_UPGRADE_MONITOR_H_

/**
 * @file QBUpgradeMonitor.h
 * @brief Upgrade monitor interface
 **/

/**
 * @defgroup QBUpgradeMonitor Upgrade monitor interface
 * @ingroup QBUpgrade
 * @{
 **/

#include <stdint.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * Upgrade monitor interface.
 **/
typedef struct QBUpgradeMonitor_ {
    /**
     * FIXME
     **/
    void (*setCheckPeriod)(SvObject self_,
                           unsigned int period);

    /**
     * FIXME
     **/
    void (*setListener)(SvObject self_,
                        SvObject listener,
                        SvErrorInfo *errorOut);

    /**
     * FIXME
     **/
    void (*start)(SvObject self_,
                  SvScheduler scheduler,
                  SvErrorInfo *errorOut);

    /**
     * FIXME
     **/
    void (*stop)(SvObject self_,
                 SvErrorInfo *errorOut);

    /**
     * FIXME
     **/
    void (*checkNow)(SvObject self_,
                     SvErrorInfo *errorOut);

    /**
     * FIXME
     **/
    void (*reset)(SvObject self_);
} *QBUpgradeMonitor;


/**
 * Get runtime type identification object
 * representing QBUpgradeMonitor interface.
 *
 * @return QBUpgradeMonitor interface object
 **/
extern SvInterface
QBUpgradeMonitor_getInterface(void);

/**
 * @}
 **/


#endif

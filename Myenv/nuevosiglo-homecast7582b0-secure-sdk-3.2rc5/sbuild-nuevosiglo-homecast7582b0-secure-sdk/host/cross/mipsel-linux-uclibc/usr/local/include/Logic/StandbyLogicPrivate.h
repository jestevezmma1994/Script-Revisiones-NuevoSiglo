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

#ifndef STANDBY_LOGIC_PRIVATE_H_
#define STANDBY_LOGIC_PRIVATE_H_

/**
 * @file StandbyLogicPrivate.h Standby logic class private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Logic/StandbyLogic.h>
#include <main_decl.h>
#include <SvEPGDataLayer/Plugins/DVBEPGPlugin.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvTime.h>
#include <stdbool.h>

/**
 * @addtogroup StandbyLogic
 * @{
 **/

/**
 * StandbyLogic class internals.
 * @private
 **/
struct QBStandbyLogic_ {
    /// super class
    struct SvObject_ super_;

    /// CubiTV application state
    AppGlobals appGlobals;

    /// application start timestamp
    SvTime appStartTime;

    /// front panel clock service handle
    SvObject frontPanelClock;

    /**
     * internal state of the standby logic: @c true after
     * QBStandbyLogicStop() was executed, otherwise @c false
     **/
    bool isStopped;

    /**
     * current standby state of the application: @c true after
     * QBStandbyLogicEnterStandby() was executed,
     * set again to @c false by QBStandbyLogicLeaveStandby()
     **/
    bool inStandby;

    /**
     * flag set to @c true to signal that application is not yet ready
     * for the STB to go to passive standby (powered off)
     **/
    bool delaySwitchToOff;

    /**
     * flag indicating that no standby happened since last power down,
     * reebot, or passive standby;
     * set to @c true on start of the service,
     * set to @c false after box leaves active standby
     **/
    bool noActiveStandbyHappened;

    /// channel update parameters for standby scanner
    DVBEPGPluginUpdateParams *updateParams;
    /// channel update parameters for standby scanner
    DVBEPGPluginMarkAllUnseenAsDeadParams *markParams;
};

/**
 * @}
 **/


#endif

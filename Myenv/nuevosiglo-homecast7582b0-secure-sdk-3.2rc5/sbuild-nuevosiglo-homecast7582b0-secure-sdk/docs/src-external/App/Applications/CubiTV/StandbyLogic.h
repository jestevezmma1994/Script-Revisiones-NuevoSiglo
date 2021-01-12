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

#ifndef STANDBY_LOGIC_H_
#define STANDBY_LOGIC_H_

/**
 * @file StandbyLogic.h Standby logic class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <Services/QBStandbyAgent.h>
#include <QBWindowContext.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvHashTable.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>


/**
 * @defgroup StandbyLogic Standby logic class
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Standby logic class.
 * @class QBStandbyLogic StandbyLogic.h <Logic/StandbyLogic.h>
 *
 * @extends SvObject
 *
 * This is an application logic class implementing functionality
 * related to entering and leaving standby state. An instance of this
 * class can be created by using @ref QBLogicFactory.
 **/
typedef struct QBStandbyLogic_ *QBStandbyLogic;

/**
 * Standby logic virtual methods table.
 **/
typedef const struct QBStandbyLogicVTable_ {
    /// virtual methods table of the base class
    struct SvObjectVTable_ super_;

    /**
     * Initialize standby logic instance.
     *
     * @param[in] self      standby logic handle
     * @param[in] appGlobals CubiTV application state
     * @param[in] pluginsWhiteList white list of standby agent plugins
     * @param[out] errorOut error info
     * @return              @a self or @c NULL in case of error
     **/
    QBStandbyLogic (*init)(QBStandbyLogic self,
                           AppGlobals appGlobals,
                           SvHashTable pluginsWhiteList,
                           SvErrorInfo *errorOut);

    /**
     * Start standby logic.
     *
     * @param[in] self      standby logic handle
     **/
    void (*start)(QBStandbyLogic self);

    /**
     * Stop standby logic.
     *
     * @param[in] self      standby logic handle
     **/
    void (*stop)(QBStandbyLogic self);

    /**
     * Handle input event received during active standby.
     *
     * @param[in] self      standby logic handle
     * @param[in] ev        input event
     * @return              @c true if @a ev was handled
     **/
    bool (*handleInputEvent)(QBStandbyLogic self,
                             SvInputEvent ev);

    /**
     * Perform all tasks needed for STB to enter standby state.
     *
     * @param[in] self      standby logic handle
     **/
    void (*enterStandby)(QBStandbyLogic self);

    /**
     * Perform all tasks needed for STB to leave standby state.
     *
     * @param[in] self      standby logic handle
     **/
    void (*leaveStandby)(QBStandbyLogic self);

    /**
     * Create and activate services that are supposed to be running
     * in active standby state.
     *
     * @param[in] self      standby logic handle
     * @param[in] context   standby window context handle
     **/
    void (*prepareServices)(QBStandbyLogic self,
                            QBWindowContext context);

    /**
     * Calculate how long STB can stay in passive standby mode before
     * waking up to perform scheduled tasks (for example: scheduled
     * recordings.)
     *
     * @param[in] self      standby logic handle
     * @return  how long to sleep, in seconds
     **/
    int (*getWakeupDelay)(QBStandbyLogic self);
    /**
     * Perform all tasks needed for STB to enter passive standby state.
     *
     * @param[in] self      standby logic handle
     **/
    void (*enterPassiveStandby)(QBStandbyLogic self);
} *QBStandbyLogicVTable;


/**
 * Get runtime type identification object representing standby logic class.
 * @return standby logic class
 **/
extern SvType
QBStandbyLogic_getType(void);


/**
 * Create an instance of standby logic using @ref QBLogicFactory.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] appGlobals CubiTV application state
 * @param[in] pluginsWhiteList white list of standby agent plugins
 * @param[out] errorOut error info
 * @return              created standby logic, @c NULL in case of error
 **/
extern QBStandbyLogic
QBStandbyLogicCreate(AppGlobals appGlobals,
                     SvHashTable pluginsWhiteList,
                     SvErrorInfo *errorOut);

/**
 * Start standby logic.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self      standby logic handle
 **/
static inline void
QBStandbyLogicStart(QBStandbyLogic self)
{
    return SvInvokeVirtual(QBStandbyLogic, self, start);
}

/**
 * Stop standby logic.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self      standby logic handle
 **/
static inline void
QBStandbyLogicStop(QBStandbyLogic self)
{
    return SvInvokeVirtual(QBStandbyLogic, self, stop);
}

/**
 * Handle input event received during active standby.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self      standby logic handle
 * @param[in] ev        input event
 * @return              @c true if @a ev was handled
 **/
static inline bool
QBStandbyLogicHandleInputEvent(QBStandbyLogic self,
                               SvInputEvent ev)
{
    return SvInvokeVirtual(QBStandbyLogic, self, handleInputEvent, ev);
}

/**
 * Perform all tasks needed for STB to enter standby state.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self      standby logic handle
 **/
static inline void
QBStandbyLogicEnterStandby(QBStandbyLogic self)
{
    return SvInvokeVirtual(QBStandbyLogic, self, enterStandby);
}

/**
 * Perform all tasks needed for STB to leave standby state.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self      standby logic handle
 **/
static inline void
QBStandbyLogicLeaveStandby(QBStandbyLogic self)
{
    return SvInvokeVirtual(QBStandbyLogic, self, leaveStandby);
}

/**
 * Perform all tasks needed for STB to enter passive standby state.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self      standby logic handle
 **/
static inline void
QBStandbyLogicEnterPassiveStandby(QBStandbyLogic self)
{
    return SvInvokeVirtual(QBStandbyLogic, self, enterPassiveStandby);
}

/**
 * Create and activate services that are supposed to be running
 * in active standby state.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self      standby logic handle
 * @param[in] context   standby window context handle
 **/
static inline void
QBStandbyLogicPrepareServices(QBStandbyLogic self,
                              QBWindowContext context)
{
    return SvInvokeVirtual(QBStandbyLogic, self, prepareServices, context);
}

/**
 * Check if switch from current to wanted state is possible.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self      standby logic handle
 * @param[in] currState current state
 * @param[in] wantedState wanted state
 * @return              @c true if switch is possible
 **/
extern bool
QBStandbyLogicCanSwitch(QBStandbyLogic self,
                        QBStandbyStateData currState,
                        QBStandbyStateData wantedState);

/**
 * Calculate how long STB can stay in passive standby mode before
 * waking up to perform scheduled tasks (for example: scheduled
 * recordings.)
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self              standby logic handle
 * @return                      how long to sleep, in seconds
 **/
static inline int
QBStandbyLogicGetWakeupDelay(QBStandbyLogic self)
{
    return SvInvokeVirtual(QBStandbyLogic, self, getWakeupDelay);
}

/**
 * Merge requested states from plugins into one final state.
 *
 * @memberof QBStandbyLogic
 *
 * @param[in] self              standby logic handle
 * @param[in] pluginsToStates   states to merge
 * @param[out] finalStateData   final state after merge
 * @return                      @c true if @a finalStateData changed, @c false otherwise
 */
extern bool
QBStandbyLogicMergeWantedStates(QBStandbyLogic self,
                                SvArray pluginsToStates,
                                QBStandbyStateData finalStateData);

/**
 * Retrieve current initial and wanted standby statuses.
 *
 * @relates QBStandbyLogic
 *
 * @param[in] previousWanted    previous wanted stated (before reboot)
 * @param[out] initial          initial standby state
 * @param[out] wanted           wanted standby state
 */
extern void
QBStandbyLogicGetStandbyStatus(QBStandbyStateData previousWanted,
                               QBStandbyStateData initial,
                               QBStandbyStateData wanted);

/**
 * @}
 **/


#endif

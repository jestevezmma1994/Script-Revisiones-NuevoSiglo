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


#ifndef QB_STANDBY_AGENT_H
#define QB_STANDBY_AGENT_H

/**
 * @file QBStandbyAgent.h
 **/

/**
 * @defgroup QBStandbyAgent Standby Agent
 * @ingroup CubiTV_services
 * @{
 **/

#include <main_decl.h>
#include <stdbool.h>

#include <fibers/c/fibers.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>

//TODO: these should really be changed to have more meaninfgul names
typedef enum QBStandbyState_e
{
    QBStandbyState_off,
    QBStandbyState_standby,
    QBStandbyState_on,
    QBStandbyState_unknown,
    QBStandbyState_cnt
} QBStandbyState;

typedef struct QBStandbyStateData_s* QBStandbyStateData;
typedef struct QBStandbyAgent_s* QBStandbyAgent;

/**
 * Create QBStandbyStateData object.
 *
 * @param[in] state          desired standby state
 * @param[in] service        object handle to source of state data
 * @param[in] popupWanted    information if popup window is wanted
 * @param[in] source         information about source of state data
 * @return created value
 **/
QBStandbyStateData
QBStandbyStateDataCreate(QBStandbyState state, SvObject service, bool popupWanted, SvString source);

/**
 * Converts AUTO_POWER string taken from configuration file (QBConfig) to QBStandbyState type
 *
 * @param[in] value an AUTO_POWER string
 * @return converted value
 **/
QBStandbyState QBStandbyStateFromAutoPowerString(const char *value);

/**
 * Set new state of object.
 *
 * @param[in] self           object handle
 * @param[in] state          new state
 * @param[in] popupWanted    new popupWanted
 * @param[in] reason         new reason
 **/
void QBStandbyStateDataSetState(QBStandbyStateData self, QBStandbyState state, bool popupWanted, SvString reason);

/**
 * Set state of of object based on source object.
 *
 * @param[in] self        object handle
 * @param[in] source      object handle to source object
 **/
void QBStandbyStateDataSetFromStateData(QBStandbyStateData self, const QBStandbyStateData source);

/**
 * Returns string representation of QBStandbyStateData object.
 *
 * @param[in] state          object handle
 *
 * @return Name of state. Value is static string and you should not release it.
 **/
const char* QBStandbyStateDataToString(const QBStandbyStateData state);

/**
 * Returns stored state.
 *
 * @param[in] state      object handle
 *
 * @return   stored state value.
 **/
QBStandbyState QBStandbyStateDataGetState(const QBStandbyStateData state);

/**
 * Returns information if state require show popup window.
 *
 * @param[in] state      object handle
 *
 * @return  want show popup window.
 **/
bool QBStandbyStateDataGetPopupWanted(const QBStandbyStateData state);

/**
 * Returns text information about reason of state.
 *
 * @param[in] state      object handle
 *
 * @return   reason of current state.
 **/
SvString QBStandbyStateDataGetReason(const QBStandbyStateData state);

/**
 * Get initial values of current and wanted state read from file.
 *
 * @deprecated This method is deprecated and you should not to use it in new code.
 * This function and usages will be refactored.
 *
 * @param[out] initial   after call of method this value will be set state from file
 * @param[out] wanted    after call of method this value will be set
 **/
void QBStandbyAgentGetInitialState(QBStandbyStateData initial, QBStandbyStateData wanted);

QBStandbyAgent QBStandbyAgentCreate(AppGlobals appGlobals);

void QBStandbyAgentStart(QBStandbyAgent self, SvScheduler scheduler);
void QBStandbyAgentStop(QBStandbyAgent self);


/**
* Returns information if application is in standby state.
*
* @param [in] self  object handle
*
* @return @c true if current state of application is standby.
*/
bool QBStandbyAgentIsStandby(const QBStandbyAgent self);

/**
 * Returns current state of QBStandbyAgent.
 *
 * @deprecated This method is deprecated and you should not to use it in new code. It
 * will be removed because in most cases users are not interested current state, but they
 * are interested that we are in standby or not. See QBStandbyAgentIsStandby().
 *
 * @param[in] standby    object handle
 *
 * @return Current state of QBStandbyAgnet.
 **/
QBStandbyStateData QBStandbyAgentGetCurrentDataState(const QBStandbyAgent standby);

void QBStandbyAgentSetWantedState(QBStandbyAgent standby, QBStandbyStateData state);

bool QBStandbyAgentIsCompleted(const QBStandbyAgent agent);
int QBStandbyAgentGetWakeupDelay(const QBStandbyAgent self);

void QBStandbyAgentAddService(const QBStandbyAgent self, SvObject service);
void QBStandbyAgentStartServices(const QBStandbyAgent agent);
void QBStandbyAgentStopServices(const QBStandbyAgent agent);

/**
 * Get runtime type identification object representing QBStandbyAgent type.
 *
 * @return QBStandbyAgent runtime type identification object
**/
SvType QBStandbyAgent_getType(void);

/**
 * @}
 **/

#endif // #ifndef QB_STANDBY_AGENT_H

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

#ifndef PVR_AGENT_H_
#define PVR_AGENT_H_

/**
 * @file pvrAgent.h
 * @brief Agent managing recordings in PVR.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPVRProviderRequest.h>
#include <QBDataModel3/QBActiveArray.h>
#include <QBDataModel3/QBActiveTree.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvFoundation/SvGenericObject.h>

#include "main_decl.h"

/**
 * @defgroup QBPVRAgent Agent managing recordings in PVR
 * @ingroup CubiTV_services
 * @{
 **/
typedef struct QBPVRAgent_s* QBPVRAgent;

/**
 * Creates new instance of agent for managing recordings in PVR.
 *
 * @param[in] appGlobals                    AppGlobals handle
 * @return                                  handle to agent for managing recordings in PVR
 **/
extern QBPVRAgent
QBPVRAgentCreate(AppGlobals appGlobals);

/**
 * Returns an array of PVR recordings that were completed.
 *
 * @param[in] self              handle to QBPVRAgent
 * @return                      array that contains completed PVR recordings
 **/
extern QBActiveArray
QBPVRAgentGetCompletedRecordingsList(QBPVRAgent self);

/**
 * Returns an array of PVR recordings that were scheduled.
 *
 * @param[in] self              handle to QBPVRAgent
 * @return                      array that contains scheduled PVR recordings
 **/
extern QBActiveArray
QBPVRAgentGetScheduledRecordingsList(QBPVRAgent self);

/**
 * Returns an array of PVR recordings that were ongoing.
 *
 * @param[in] self              handle to QBPVRAgent
 * @return                      array that contains ongoing PVR recordings
 **/
extern QBActiveArray
QBPVRAgentGetOnGoingRecordingsList(QBPVRAgent self);

/**
 * Returns an array of PVR recordings that were failed.
 *
 * @param[in] self              handle to QBPVRAgent
 * @return                      array that contains failed PVR recordings
 **/
extern QBActiveArray
QBPVRAgentGetFailedRecordingsList(QBPVRAgent self);

/**
 * Return array of the removed PVR recordings.
 *
 * @param[in] self              handle to QBPVRAgent
 * @return                      QBPVRRecording array that contains removed recordings
 **/
extern QBActiveArray
QBPVRAgentGetRemovedRecordingsList(QBPVRAgent self);

/**
 * Starts agent for managing recordings in PVR.
 *
 * @param[in] self      handle to QBPVRAgent
 **/
extern void
QBPVRAgentStart(QBPVRAgent self);

/**
 * Stops agent for managing recordings in PVR.
 *
 * @param[in] self      handle to QBPVRAgent
 **/
extern void
QBPVRAgentStop(QBPVRAgent self);

/**
 * Marks a PVR request as tracked. If a request is tracked then in case of it's failure EPG event
 * is refreshed to make sure it is up to date, and to eliminate outdated EPG event as a possible reason for
 * request failures.
 *
 * @param[in] self      QBPVRAgent handle
 * @param[in] request   request that is to be tracked
 * @param[in] event     event which will be refreshed when request fails
 */
extern void
QBPVRAgentTrackRequest(QBPVRAgent self, QBPVRProviderRequest request, SvEPGEvent event);

/**
 * @}
 **/

#endif /* PVR_AGENT_H_ */

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

#ifndef NPVR_AGENT_H_
#define NPVR_AGENT_H_

/**
 * @file npvrAgent.h
 * @brief Agent managing recordings in NPvr.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "main_decl.h"

#include <SvFoundation/SvGenericObject.h>
#include <QBDataModel3/QBActiveArray.h>
#include <QBDataModel3/QBActiveTree.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <NPvr/QBnPVRProviderRequest.h>

/**
 * @defgroup QBNPvrAgent Agent managing recordings in NPvr
 * @ingroup CubiTV_services
 * @{
 **/
typedef struct QBNPvrAgent_s* QBNPvrAgent;

/**
 * Creates new instance of agent for managing recordings in NPvr.
 *
 * @param[in] appGlobals                    AppGlobals handle
 * @param[in] npvrMenuRefreshDelaySec       NPvr recordings refresh period in seconds
 * @return                                  handle to agent for managing recordings in NPvr
 **/
extern QBNPvrAgent
QBNPvrAgentCreate(AppGlobals appGlobals, unsigned int npvrMenuRefreshDelaySec);

/**
 * Returns array of NPvr recordings that where completed.
 *
 * @param[in] self              handle to QBNPvrAgent
 * @return                      array that contains completed NPvr recordings
 **/
extern QBActiveArray
QBNPvrAgentGetCompletedRecordingsList(QBNPvrAgent self);

/**
 * Returns array of NPvr recordings that where scheduled.
 *
 * @param[in] self              handle to QBNPvrAgent
 * @return                      array that contains scheduled NPvr recordings
 **/
extern QBActiveArray
QBNPvrAgentGetScheduledRecordingsList(QBNPvrAgent self);

/**
 * Returns array of NPvr recordings that where ongoing.
 *
 * @param[in] self              handle to QBNPvrAgent
 * @return                      array that contains ongoing NPvr recordings
 **/
extern QBActiveArray
QBNPvrAgentGetOnGoingRecordingsList(QBNPvrAgent self);

/**
 * Returns array of NPvr recordings that where failed.
 *
 * @param[in] self              handle to QBNPvrAgent
 * @return                      array that contains failed NPvr recordings
 **/
extern QBActiveArray
QBNPvrAgentGetFailedRecordingsList(QBNPvrAgent self);

/**
 * Return array of the removed NPVR recordings.
 *
 * @param[in] self              handle to QBNPvrAgent
 * @return                      QBnPVRRecording array that contains removed recordings
 **/
extern QBActiveArray
QBNPvrAgentGetRemovedRecordingsList(QBNPvrAgent self);

/**
 * Starts agent for managing recordings in NPvr.
 *
 * @param[in] self      handle to QBNPvrAgent
 * @param[in] menuTree  handle to menu data source
 * @param[in] menuPath  handle to path with Npvr menu
 **/
extern void
QBNPvrAgentStart(QBNPvrAgent self,
                 QBActiveTree menuTree,
                 SvGenericObject menuPath);

/**
 * Stops agent for managing recordings in NPvr.
 *
 * @param[in] self      handle to QBNPvrAgent
 **/
extern void
QBNPvrAgentStop(QBNPvrAgent self);

/**
 * Marks a nPVR request as tracked. If a request is tracked then in case of it's failure EPG event
 * is refreshed to make sure it is up to date, and to eliminate outdated EPG event as a possible reason for
 * request failures.
 *
 * @param[in] self      QBNPvrAgent handle
 * @param[in] request   request that is to be tracked
 * @param[in] event     event which will be refreshed when request fails
 */
extern void
QBNPvrAgentTrackRequest(QBNPvrAgent self, QBnPVRProviderRequest request, SvEPGEvent event);

/**
 * @}
 **/

#endif /* NPVR_AGENT_H_ */

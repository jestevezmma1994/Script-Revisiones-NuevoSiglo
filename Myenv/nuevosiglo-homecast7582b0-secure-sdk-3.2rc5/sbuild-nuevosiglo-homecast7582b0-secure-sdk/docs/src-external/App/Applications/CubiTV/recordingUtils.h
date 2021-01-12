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

#ifndef QB_RECORDING_UTILS_H_
#define QB_RECORDING_UTILS_H_

#include <QBPVRProvider.h>
#include <QBPVRRecording.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>

#include <main_decl.h>
#include <stdbool.h>

typedef enum {
    //Recording types
    QBRecordingUtilsRecType_Keyword,    //Recording scheduled for event containing specified keyword (by repeated recordings manager)
    QBRecordingUtilsRecType_Repeat,     //Recording scheduled for channel and time interval (by repeated recordings manager)
    QBRecordingUtilsRecType_Event,      //Recording scheduled for event
    QBRecordingUtilsRecType_Channel,    //Recording scheduled for channel
    QBRecordingUtilsRecType_Manual,     //Recording scheduled for channel and time interval

    QBRecordingUtilsRecType_Single,     //Recording which isn't managed by repeated recordings manager
    QBRecordingUtilsRecType_Multiple,   //Recording which is managed by repeated recordings manager

    //Recording states
    QBRecordingUtilsRecType_Scheduled,  //Scheduled (but not started) recording
    QBRecordingUtilsRecType_Active,     //Ongoing recording
    QBRecordingUtilsRecType_Completed,  //Successful finished recording
    QBRecordingUtilsRecType_Missed,     //Missed recording
    QBRecordingUtilsRecType_Failed,     //Failed recording
    QBRecordingUtilsRecType_Removed,    //Removed recording

    QBRecordingUtilsRecType_Any         //Recording of any type/state
} QBRecordingUtilsRecType;

/**
 * Create name for recording
 *
 * @param[in] appGlobals        application globals
 * @param[in] recording         recording handle
 * @param[in] event             add event information
 * @param[in] status            add status information
 * @return                      name
 */
SvString
QBRecordingUtilsCreateName(AppGlobals appGlobals, const QBPVRRecording recording, bool event, bool status);

/**
 * Create time information for recording
 *
 * @param[in] pvrProvider       PVR provider
 * @param[in] recording         recording handle
 * @return                      time information
 */
SvString
QBRecordingUtilsCreateTimeInformation(QBPVRProvider pvrProvider,
                                      QBPVRRecording recording);

/**
 * Create not completed recordings array for given event
 *
 * @param[in] provider          PVR provider
 * @param[in] event             event handle
 * @return                      time information
 */
SvArray
QBRecordingUtilsCreateNotCompletedRecordingsForEvent(QBPVRProvider provider, SvEPGEvent event);

/**
 * Get recording with given type from recordings array
 *
 * @param[in] rec               recordings
 * @param[in] recType           wanted recording type
 * @return                      recording of given type, @c NULL if not found
 */
QBPVRRecording
QBRecordingUtilsGetRecordingOfType(SvArray rec, QBRecordingUtilsRecType recType);

/**
 * Stop recording for event
 *
 * @param[in] provider          PVR provider
 * @param[in] event             event for which recording should be stopped
 * @return                      @c true if successful, @c false otherwise
 */
bool
QBRecordingUtilsStopRecordingForEvent(QBPVRProvider provider, SvEPGEvent event);

/**
 * Test if recording matches search criteria
 *
 * @param[in] appGlobals        application globals
 * @param[in] recording         recording to be tested
 * @param[in] keywords          keywords
 * @return                      @c true if recording matches search criteria, @c false otherwise
 */
bool
QBRecordingUtilsPVRSearchProviderTest(void *appGlobals, SvObject recording, SvArray keywords);

#endif //QB_RECORDING_UTILS_H_

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

#ifndef QBOSDRECORDING_H_
#define QBOSDRECORDING_H_

/**
 * @file QBOSDRecording.h Base class for OSD recordings components
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBOSD/QBOSD.h>

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerManager/SvPlayerTaskState.h>
#include <SvPlayerKit/SvEPGEvent.h>

#include <time.h>

/**
 * @defgroup QBOSDRecording Base class for OSD recordings components
 * @ingroup QBOSD
 * @{
 *
 * An abstract base class for OSD recordings components.
 **/

/**
 * An abstract base class for OSD recordings components.
 *
 * @class QBOSDRecording QBOSDRecording.h <QBOSD/QBOSDRecording.h>
 * @extends QBOSD
 **/
typedef struct QBOSDRecording_ *QBOSDRecording;

/**
 * @relates QBOSDRecording
 *
 * OSD recordings class internals.
 */
struct QBOSDRecording_ {
    /**
     * super class
     **/
    struct QBOSD_ super_;
};

/**
 * @relates QBOSDRecording
 *
 * Callback used to retrieve current/next event from given channel.
 *
 * @param[in]  data         callback's private data
 * @param[in]  channel      event source
 * @param[in]  currentTime  current time
 * @param[out] current      current event
 * @param[out] next         next event
 **/
typedef void (*QBOSDTimeshiftEventSource)(void *data,
                                          SvTVChannel channel,
                                          time_t currentTime,
                                          SvEPGEvent *current,
                                          SvEPGEvent *next);

/**
 * @relates QBOSDRecording
 *
 * QBOSDRecording virtual methods table
 */
typedef struct QBOSDRecordingVTable_ {
    /**
     * SvObject virtualmethods table
     */
    struct QBOSDVTable_ super_;

    /**
     * Set event source callback.
     *
     * @public @memberof QBOSDRecording
     *
     * Use this function internally, to retrieve current/next event when needed
     * (eg. inside QBOSDRecordingUpdateTimes()).
     *
     * @param[in]  self     OSD recordings object handle
     * @param[in]  source   event source callback
     * @param[in]  userData private data passed to event source function
     * @param[out] errorOut error info
     **/
    void (*setEventSource)(QBOSDRecording self,
                           QBOSDTimeshiftEventSource source,
                           void *userdata,
                           SvErrorInfo *errorOut);

    /**
     * Update OSD widget according to given time values.
     *
     * @note If the current/next event storing is needed, use this function to
     *       update it's values.
     *
     * @param[in]  self           OSD recordings object handle
     * @param[in]  channel        handle to channel being recorded
     * @param[in]  currentTime    current time
     * @param[in]  timeshiftStart timeshift start time
     * @param[in]  timeshiftEnd   timeshift end time
     * @param[out] errorOut       error info
     **/
    void (*updateTimes)(QBOSDRecording self,
                        SvTVChannel channel,
                        time_t currentTime,
                        time_t recordingStart,
                        time_t recordingEnd,
                        SvErrorInfo *errorOut);

    /**
     * Update events information on OSD widget.
     *
     * This function is called when events need to be refreshed
     * (eg. when current event changed or after window reinitialization)
     *
     * @param[in]  self     OSD recordings object handle
     * @param[out] errorOut error info
     **/
    void (*updateEvents)(QBOSDRecording self,
                         SvErrorInfo *errorOut);

    /**
     * Update OSD widgets according to given playback state.
     *
     * @param[in]  self        OSD recordings object handle
     * @param[in]  state       state of player task
     * @param[in]  wantedSpeed speed requested by user
     * @param[out] errorOut    error info
     **/
    void (*updatePlaybackState)(QBOSDRecording self,
                                const SvPlayerTaskState *state,
                                double wantedSpeed,
                                SvErrorInfo *errorOut);

    /**
     * Update OSD widget according to given timeshift information
     *
     * @param[in]  self                OSD recording object handle
     * @param[in]  availabilityTime    timeshift availability time
     * @param[in]  isAvailable         @c true if timeshift is available, @c false if not
     * @param[out] errorOut            error info
     **/
    void (*setAvailabilityLimit)(QBOSDRecording self,
                                 SvTime availabilityTime,
                                 bool isAvailable,
                                 SvErrorInfo *errorOut);
} *QBOSDRecordingVTable;

/**
 * @relates QBOSDRecording
 *
 * Get runtime type identification object representing OSD recording class.
 *
 * @return QBOSDRecording type identification object
 **/
SvType
QBOSDRecording_getType(void);

/**
 * Initialize OSD recording object.
 *
 * @protected @memberof QBOSDRecording
 *
 * @note Internally, this method calls @ref QBOSDInit().
 *
 * @param[in]  self     OSD recordings object handle
 * @param[in]  w        underlying widget
 * @param[in]  id       widget identifier
 * @param[out] errorOut error info
 **/
void
QBOSDRecordingInit(QBOSDRecording self,
                   SvWidget w,
                   SvString id,
                   SvErrorInfo *errorOut);

/**
 * Set event source callback.
 *
 * @public @memberof QBOSDRecording
 *
 * Use this function internally, to retrieve current/next event when needed
 * (eg. inside QBOSDRecordingUpdateTimes()).
 *
 * @param[in]  self     OSD recordings object handle
 * @param[in]  source   event source callback
 * @param[in]  userData private data passed to event source function
 * @param[out] errorOut error info
 **/
void
QBOSDRecordingSetEventSource(QBOSDRecording self,
                             QBOSDTimeshiftEventSource source,
                             void *userData,
                             SvErrorInfo *errorOut);

/**
 * Update OSD widget according to given time values.
 *
 * @public @memberof QBOSDRecording
 *
 * @note If the current/next event storing is needed, use this function to
 *       update it's values.
 *
 * @param[in]  self           OSD recordings object handle
 * @param[in]  channel        handle to channel being recorded
 * @param[in]  currentTime    current time
 * @param[in]  timeshiftStart timeshift start time
 * @param[in]  timeshiftEnd   timeshift end time
 * @param[out] errorOut       error info
 **/
void
QBOSDRecordingUpdateTimes(QBOSDRecording self,
                          SvTVChannel channel,
                          time_t currentTime,
                          time_t timeshiftStart,
                          time_t timeshiftEnd,
                          SvErrorInfo *errorOut);

/**
 * Update events information on OSD widget.
 *
 * @public @memberof QBOSDRecording
 *
 * This function is called when events need to be refreshed
 * (eg. when current event changed or after window reinitialization)
 *
 * @param[in]  self     OSD recordings object handle
 * @param[out] errorOut error info
 **/
void
QBOSDRecordingUpdateEvents(QBOSDRecording self,
                           SvErrorInfo *errorOut);

/**
 * Update OSD widgets according to given playback state.
 *
 * @public @memberof QBOSDRecording
 *
 * @param[in]  self        OSD recordings object handle
 * @param[in]  state       state of player task
 * @param[in]  wantedSpeed speed requested by user
 * @param[out] errorOut    error info
 **/
void
QBOSDRecordingUpdatePlaybackState(QBOSDRecording self,
                                  const SvPlayerTaskState *state,
                                  double wantedSpeed,
                                  SvErrorInfo *errorOut);


/**
 * Update OSD widget according to given timeshift information
 *
 * @param[in]  self                OSD recording object handle
 * @param[in]  availabilityTime    timeshift availability time
 * @param[in]  isAvailable         @c true if timeshift is available, @c false if not
 * @param[out] errorOut            error info
 **/
void
QBOSDRecordingSetAvailabilityLimit(QBOSDRecording self,
                                   SvTime availabilityTime,
                                   bool isAvailable,
                                   SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBOSDRECORDING_H_ */

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

#ifndef QBSMOOTHSTREAMINGMEDIAINFO_H_
#define QBSMOOTHSTREAMINGMEDIAINFO_H_

#include <SmoothStreaming/types.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <fibers/c/fibers.h>
#include <storage_input/storage_input.h>
#include <SvPlayerKit/SvChbuf.h>

/**
 * @file  QBSmoothStreamingMediaInfo.h
 * @brief Smooth Streaming Media Information Service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBSmoothStreamingMediaInfo Smooth Streaming Media information class
 * @ingroup containers
 * @{
 **/
typedef struct QBSMoothStreamingMediaInfo_ * QBSMoothStreamingMediaInfo;

struct QBSmoothStreamingMediaInfoCallbacks {
    /**
     * Notify that an error has occurred while downloading manifest
     *
     * @param[in] owner             pointer to owner of callbacks
     * @param[in] errorCode         error code
     **/
    void (*manifestError)(void* owner, int errorCode);

    /**
     * Notify that the first manifest download was successfull and data is available
     *
     * @param[in] owner             pointer to owner of callbacks
     * @param[in] smooth_media      media information structure
     **/
    void (*dataAvailable)(void* owner, MSSMedia smooth_media);

    /**
     * Notify that the manifest was refreshed
     *
     * @param[in] owner             pointer to owner of callbacks
     **/
    void (*manifestRefreshed)(void* owner);

    /**
     * Notify that the livepoint was moved due to dvr window simulation or due to manifest refresh
     *
     * @param[in] owner             pointer to owner of callbacks
     **/
    void (*livepointMoved)(void* owner);
};

typedef struct QBSmoothStreamingMediaInfoCallbacks QBSmoothStreamingMediaInfoCallbacks;

/**
 * Create Smooth Streaming Media Info.
 *
 * @param[in] owner             owner of media info
 * @param[in] httpEngine        HTTP client engine
 * @param[in] callbacks         callbacks that will be called when appropriate
 * @param[in] scheduler         sheduler handle
 * @param[in] parent_input      input which allready started manifest download
 * @param[in] leftover          data allready recieved from the parent_input
 * @return                      new Smooth Streaming Media Info instance,
 *                              or @c NULL in case of error
 **/
QBSMoothStreamingMediaInfo
QBSMoothStreamingMediaInfoCreate(void* owner, SvHTTPClientEngine httpEngine, QBSmoothStreamingMediaInfoCallbacks callbacks,
                                 SvScheduler scheduler, struct storage_input *parent_input, SvChbuf leftover);

/**
 * Enable Manifest refreshing.
 * Manifest refreshing is crucial when we are not able to obtain next chunk data form
 * chunk that was allready downloaded.
 *
 * @param[in] self          this object handle
 * @param[in] refreshSecs   period of time between consecutive manifest refreshes
 *
 **/
void
QBSMoothStreamingMediaInfoEnableManifestRefresh(QBSMoothStreamingMediaInfo self, double refreshSecs);

/**
 * Disable Manifest refreshing
 *
 * @param[in] self          this object handle
 *
 **/
void
QBSMoothStreamingMediaInfoDisableManifestRefresh(QBSMoothStreamingMediaInfo self);

/**
 * Enable DVR window simulation.
 * After this method is called the DVR window movement will be simulated.
 * Every refreshSecs the window data will be updated and the owner will be notified.
 * This lets us know where the window is, even when we are not refreshing the manifest.
 *
 * @param[in] self          this object handle
 * @param[in] refreshSecs   period of time between consecutive DVR window simulation steps
 *
 **/
void
QBSMoothStreamingMediaInfoEnableWindowSimulation(QBSMoothStreamingMediaInfo self, double refreshSecs);

/**
 * Disable DVR window simulation.
 *
 * @param[in] self          this object handle
 *
 **/
void
QBSmoothStreamingMediaInfoDisableWindowSimulation(QBSMoothStreamingMediaInfo self);

/**
 * This method tells if the first manifest was allready downloaded and parsed.
 *
 * @param[in] self          this object handle
 * @return                  @c true if the manifest was allready downloaded and @c false otherwise
 **/
bool
QBSmoothStreamingMediaInfoIsReady(QBSMoothStreamingMediaInfo self);

/**
 * This method tells if the media represented by the manifest is in inflight mode
 *
 * @param[in] self          this object handle
 * @return                  @c true if the media is inflight and @c false otherwise
 **/
bool
QBSmoothStreamingMediaInfoIsInflight(QBSMoothStreamingMediaInfo self);

/**
 * This method returns the absolute current livepoint in seconds.
 *
 * @param[in] self          this object handle
 * @return                  current livepoint in seconds (absolute value)
 **/
double
QBSMoothStreamingMediaInfoGetLivePoint(QBSMoothStreamingMediaInfo self);

/**
 * This method returns the absolute current DVR window start in seconds.
 *
 * @param[in] self          this object handle
 * @return                  current DVR window start in seconds (absolute value)
 **/
double
QBSMoothStreamingMediaInfoGetDvrWindowStart(QBSMoothStreamingMediaInfo self);

/**
 * This method returns the absolute current DVR window end in seconds.
 *
 * @param[in] self          this object handle
 * @return                  current DVR window end in seconds (absolute value)
 **/
double
QBSMoothStreamingMediaInfoGetDvrWindowEnd(QBSMoothStreamingMediaInfo self);

/**
 * This method returns the absolute current DVR window end in seconds basing only on data from manifest.
 * This means that even if the window simulation is enabled the data returned from this function is
 * the window end calculated during last manifest refresh (or first download if no refresh occurred)
 *
 * @param[in] self          this object handle
 * @return                  current DVR window end from manifest data in seconds (absolute value)
 **/
double
QBSMoothStreamingMediaInfoGetRealDvrWindowEnd(QBSMoothStreamingMediaInfo self);

/**
 * This method returns the absolute current livepoint in seconds basing only on data from manifest.
 * This means that even if the window simulation is enabled the data returned from this function is
 * the livepont calculated during last manifest refresh (or first download if no refresh occurred)
 *
 * @param[in] self          this object handle
 * @return                  current livepoint from manifest data in seconds (absolute value)
 **/
double
QBSMoothStreamingMediaInfoGetRealLivePoint(QBSMoothStreamingMediaInfo self);

/**
 * This method returns the amount of lookahed in seconds.
 * Lookahead is the difference between DVR window end and livepoint.
 *
 * @param[in] self          this object handle
 * @return                  current lookahead in seconds.
 **/
double
QBSMoothStreamingMediaInfoGetDvrWindowLookahead(QBSMoothStreamingMediaInfo self);

/**
 * This method returns the absolute media start in seconds.
 *
 * @param[in] self          this object handle
 * @return                  media start in seconds (absolute value)
 **/
double
QBSMoothStreamingMediaInfoGetMediaStart(QBSMoothStreamingMediaInfo self);

/**
 * This method returns the absolute media end in seconds.
 *
 * @param[in] self          this object handle
 * @return                  media end in seconds (absolute value)
 **/
double
QBSMoothStreamingMediaInfoGetMediaEnd(QBSMoothStreamingMediaInfo self);

/**
 * @}
 **/


#endif //QBSMOOTHSTREAMINGDOWNLOADER_H_

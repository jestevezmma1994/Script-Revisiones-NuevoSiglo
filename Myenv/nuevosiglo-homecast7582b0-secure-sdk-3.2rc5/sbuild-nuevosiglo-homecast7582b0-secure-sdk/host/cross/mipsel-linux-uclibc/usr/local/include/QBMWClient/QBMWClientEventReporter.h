/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_MW_CLIENT_EVENT_REPORTER_H_
#define QB_MW_CLIENT_EVENT_REPORTER_H_

/**
 * @file QBMWClientEventReporter.h Event reporter interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvInterface.h>
#include <fibers/c/fibers.h>

/**
 * @defgroup QBMWClientEventReporter
 * @ingroup QBMWClient
 * @{
 *
 * QBMWEventRepoter class sends information about occurred events
 * (e.g. when some item has been liked or disliked). Single instance of
 * QBMWEventRepoter for all its life is related to the same item.
 **/

/**
 * Maximum number of elements on watchlist
 **/
#define MAX_WATCHLIST_ELEMENTS 30

/**
 * Types of possible events
 **/
typedef enum {
    /** some item has been liked **/
    QBMWClientEventReporter_like,
    /** some item has been disliked **/
    QBMWClientEventReporter_dislike,
    /** clear status of item - item again will be in neutral state **/
    QBMWClientEventReporter_clearLike,
    /** add item to myList **/
    QBMWClientEventReporter_addToMyList,
    /** remove item from myList **/
    QBMWClientEventReporter_removeFromMyList,
    /** add item to recenty watched list **/
    QBMWClientEventReporter_addToWatched,
    /** implicit event - product has been rented **/
    QBMWClientEventReporter_purchase,
    /** implicit event - details of asset have been viewed **/
    QBMWClientEventReporter_viewDetails,
    /** implicit event - movie has been played **/
    QBMWClientEventReporter_play,
    /** implicit event - trailer has been played **/
    QBMWClientEventReporter_playTrailer,
    /** fetch content of my list to internal state of event reporter **/
    QBMWClientEventReporter_internal_readMyList,
    /** fetch content of watched list to internal state of event reporter **/
    QBMWClientEventReporter_internal_readWatchedList,
    /** implicit event - movie is considered watched **/
    QBMWClientEventReporter_watched
} QBMWClientEventReporterEventType;

/**
 * QBMW client event reporter request status.
 **/
typedef enum {
    /** request has not been initialized yet **/
    QBMWClientEventReporterState_init = 0,
    /** request is redy to be nequeued**/
    QBMWClientEventReporterState_readyToEnqueue,
    /** request is redy to send **/
    QBMWClientEventReporterState_ready,
    /** request is in progress **/
    QBMWClientEventReporterState_inProgress,
    /** request has been cancelled **/
    QBMWClientEventReporterState_cancelled,
    /** request has been finished withot any errors **/
    QBMWClientEventReporterState_finished,
    /** some error occurred **/
    QBMWClientEventReporterState_error
} QBMWClientEventReporterState;

/**
 * Event reporter interface.
 **/
struct QBMWClientEventReporter_ {
    /**
     * Set request listener.
     *
     * @param[in] self_     event reporter handle
     * @param[in] listener  event reporter listener handle
     * @param[out] errorOut error info
     **/
    void (*setListener)(SvObject self_,
                        SvObject listener,
                        SvErrorInfo *errorOut);

    /**
     * Send notification about event.
     *
     * @param[in] self_     event reporter handle
     * @param[in] scheduler handle to a scheduler to be used by the request
     * @param[in] eventType type of event to report
     * @param[out] errorOut error info
     **/
    int (*sendEvent)(SvObject self_, SvScheduler scheduler, QBMWClientEventReporterEventType eventType, SvErrorInfo *errorOut);

    /**
     * Stop all requests.
     *
     * @param[in] self_     event reporter handle
     * @param[out] errorOut error info
     **/
    int (*stop)(SvObject self_, SvErrorInfo *errorOut);
};

/**
 * Event reporter interface class.
 **/
typedef struct QBMWClientEventReporter_ *QBMWClientEventReporter;

/**
 * Get runtime type identification object representing
 * event reporter interface.
 * @return Event submitter interface identification object
 **/
extern SvInterface
QBMWClientEventReporter_getInterface(void);

/**
 * QBMWClient event reporter listener interface.
 **/
struct QBMWClientEventReporterListener_ {
    /**
     * Request state change notification.
     *
     * @param[in] self_         listener handle
     * @param[in] req_          request handle
     * @param[in] eventType     type of event
     * @param[in] state         request state
     **/
    void (* requestStateChanged)(SvObject self_, SvObject req_, QBMWClientEventReporterEventType eventType,
                                 QBMWClientEventReporterState state);
};

/**
 * Event reporter litener interface class.
 **/
typedef struct QBMWClientEventReporterListener_ *QBMWClientEventReporterListener;

/**
 * Get runtime type identification object representing
 * QBMWClient event reporter listener interface.
 * @return QBMWClient event reporter listener interface identification object
 **/
SvInterface
QBMWClientEventReporterListener_getInterface(void);
/**
 * @}
 **/

#endif

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


#ifndef QBEPGWATCHER_H
#define QBEPGWATCHER_H

/**
 * @file epgWatcher.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief EPG watcher class API
 **/

#include <SvEPGDataLayer/Views/SvEPGView.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBEPGWatcher EPG watcher class
 * @brief Services
 *
 * epgWatcher is used by application for getting information about
 * present and following events for available channels. For example
 * this information about events is presented on bottom OSD.
 * So the main task of this service is updating events for present
 * channels and getting information about events for newly found channels.
 *
 * @{
 **/

/**
 * Get runtime type identification object representing
 * EPG watcher listener interface
 *
 * @return QBEPGWatcherListener interface object
 */
SvInterface
QBEPGWatcherListener_getInterface(void);

/**
 * QBEPGWatcherListener interface
 **/
typedef struct QBEPGWatcherListener_t {
    /**
     * Notify that channel EPG events were changed.
     *
     * @param[in] self_     EPG watcher listener handle
     * @param[in] channel   channel events belong to
     * @param[in] current   present EPG event
     * @param[in] next      following EPG event
     **/
    void (*eventsChanged)(SvGenericObject self_, SvTVChannel channel, SvEPGEvent current, SvEPGEvent next);

    /**
     * Notify that channel EPG events were unchanged.
     *
     * @param[in] self_     EPG watcher listener handle
     * @param[in] channel   channel events belong to
     * @param[in] current   present EPG event
     * @param[in] next      following EPG event
     **/
    void (*eventsUnchanged)(SvGenericObject self_, SvTVChannel channel, SvEPGEvent current, SvEPGEvent next);
} *QBEPGWatcherListener;

/**
 * EPG watcher class
 * @class QBEPGWatcher epgWatcher.h <Services/epgWatcher.h>
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 *
 * This class expects to be initialized by @link QBInitializable @endlink
 * interface init method and has following dependencies that shall be passed in
 * properties map:
 *
 * | Name            | Type               Description
 * | --------------- | ----------------- | ------------------------------------------
 * | master          | SvEPGView         | master EPG view with all channels
 **/
typedef struct QBEPGWatcher_ *QBEPGWatcher;

/**
 * Get runtime type identification object representing QBEPGWatcher class.
 *
 * @relates QBEPGWatcher
 *
 * @return QBEPGWatcher type identification object
 */
SvType
QBEPGWatcher_getType(void);

/**
 * Register new EPG watcher listener.
 *
 * @param[in] self      EPG watcher handle
 * @param[in] listener  handle to an object implementing
 *                      QBEPGWatcherListener interface
 **/
void QBEPGWatcherAddListener(QBEPGWatcher self, SvGenericObject listener);

/**
 * Unregister EPG watcher listener.
 *
 * @param[in] self      EPG watcher handle
 * @param[in] listener  handle to an object implementing
 *                      QBEPGWatcherListener interface
 **/
void QBEPGWatcherRemoveListener(QBEPGWatcher self, SvObject listener);

/**
 * Add a channel view to watched list of channels.
 *
 * @param[in] self      EPG watcher handle
 * @param[in] list      EPG channel view handle
 **/
void QBEPGWatcherRemoveList(QBEPGWatcher self, SvEPGView list);

/**
 * Remove a channel view from watched list of channels.
 *
 * @param[in] self      EPG watcher handle
 * @param[in] list      EPG channel view handle
 **/
void QBEPGWatcherAddList(QBEPGWatcher self, SvEPGView list);

/**
 * Enable EPG watcher.
 *
 * @param[in] self      EPG watcher handle
 **/
void QBEPGWatcherEnable(QBEPGWatcher self);

/**
 * Disable EPG watcher.
 *
 * @param[in] self      EPG watcher handle
 **/
void QBEPGWatcherDisable(QBEPGWatcher self);

/**
 * @}
 **/

#endif /* QBEPGWATCHER_H_ */

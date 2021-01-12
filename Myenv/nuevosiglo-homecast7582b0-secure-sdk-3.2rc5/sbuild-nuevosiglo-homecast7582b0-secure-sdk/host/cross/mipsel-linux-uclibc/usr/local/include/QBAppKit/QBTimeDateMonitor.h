/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TIME_DATE_MONITOR_H_
#define QB_TIME_DATE_MONITOR_H_

/**
 * @file QBTimeDateMonitor.h
 * @brief Time-date monitor service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTimeDateMonitor Time date monitor service
 * @ingroup QBAppKit
 * @{
 *
 * @link QBTimeDateMonitor @endlink is a service that provides information
 * about validity of current system (wall clock) time.
 *
 * To obtain the default instance use QBServiceRegistryGetService()
 * method with well-known name "TimeDateMonitor".
 **/

/**
 * Time date monitor class.
 * @class QBTimeDateMonitor QBTimeDateMonitor.h <QBAppKit/QBTimeDateMonitor.h>
 * @extends SvObject
 * @implements QBInitializable
 * @implements QBAsyncService
 **/
typedef struct QBTimeDateMonitor_ *QBTimeDateMonitor;

/**
 * Source of system time update.
 **/
typedef enum {
    QBTimeDateUpdateSource_DVB,     ///< update from DVB TDT/TOT table
    QBTimeDateUpdateSource_NTP,     ///< IP network update (Network Time Protocol)
} QBTimeDateUpdateSource;

/**
 * Time date monitor listener interface.
 **/
typedef const struct QBTimeDateMonitorListener_ {
    /**
     * Notify that system time has been set.
     *
     * @param[in] self_     handle to an object implementing @ref QBTimeDateMonitorListener
     * @param[in] firstTime @c true if system time has been set for the first time
     * @param[in] source    source of system time information
     **/
    void (*systemTimeSet)(SvObject self_,
                          bool firstTime,
                          QBTimeDateUpdateSource source);
} *QBTimeDateMonitorListener;

/**
 * Get runtime type identification object representing
 * QBTimeDateMonitorListener interface.
 *
 * @return QBTimeDateMonitorListener interface object
 **/
extern SvInterface
QBTimeDateMonitorListener_getInterface(void);

/**
 * Get runtime type identification object representing QBTimeDateMonitor class.
 *
 * @relates QBTimeDateMonitor
 *
 * @return time date monitor class
 **/
extern SvType
QBTimeDateMonitor_getType(void);

/**
 * Check if current system (wall clock) time is valid.
 *
 * @memberof QBTimeDateMonitor
 *
 * @param[in] self      time date monitor handle
 * @return              @c true if system time is valid, @c false otherwise
 */
extern bool
QBTimeDateMonitorIsSystemTimeValid(QBTimeDateMonitor self);

/**
 * Add time date monitor listener.
 *
 * Add listener object that will be notified every time system time
 * is changed.
 *
 * @memberof QBTimeDateMonitor
 *
 * @param[in] self      time date monitor handle
 * @param[in] listener  handle to an object implementing @ref QBTimeDateMonitorListener
 * @param[out] errorOut error info
 **/
extern void
QBTimeDateMonitorAddListener(QBTimeDateMonitor self,
                             SvObject listener,
                             SvErrorInfo *errorOut);

/**
 * Remove previously registered time date monitor listener.
 *
 * @see QBTimeDateMonitorAddListener().
 *
 * @memberof QBTimeDateMonitor
 *
 * @param[in] self      time date monitor handle
 * @param[in] listener  handle to a listener registered previously using QBTimeDateMonitorAddListener()
 * @param[out] errorOut error info
 **/
extern void
QBTimeDateMonitorRemoveListener(QBTimeDateMonitor self,
                                SvObject listener,
                                SvErrorInfo *errorOut);

/**
 * Notify listeners that updated time/date information from DVB network
 * has been received.
 *
 * @param[in] self      time date monitor handle
 * @param[out] errorOut error info
 **/
extern void
QBTimeDateMonitorDVBTimeUpdated(QBTimeDateMonitor self,
                                SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

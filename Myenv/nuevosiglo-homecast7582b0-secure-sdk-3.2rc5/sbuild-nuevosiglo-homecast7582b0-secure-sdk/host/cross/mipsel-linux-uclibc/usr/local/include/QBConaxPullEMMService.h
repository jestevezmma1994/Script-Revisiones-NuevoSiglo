/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBCONAXPULLEMMSERVICE_H_
#define QBCONAXPULLEMMSERVICE_H_

/**
 * @file QBConaxPullEMMService.h Conax EMM pull service
 **/

/**
 * @defgroup QBConaxPullEMMService Service for performing Conax EMM pull
 * @ingroup CubiTV_services
 * @{
 */

#include "QBConaxPullEMMServiceDefs.h"
#include <SvFoundation/SvString.h>
#include <fibers/c/fibers.h>
#include <stdbool.h>
#include <QBSmartCardMonitor.h>

/**
 * There are two usage scenarios:
 *
 * 1. Using config server:
 * QBConaxPullEMMServiceCreate(...)
 * QBConaxPullEMMServiceSetPullConfigServerUrl(...)
 * [OPTIONAL] QBConaxPullEMMServiceSetPullConfigFile(...)
 * [OPTIONAL] QBConaxPullEMMServiceSetRequestTokenFile(...)
 * [OPTIONAL] QBConaxPullEMMServiceSetSheduledPullAfterMinTime(...)
 * [OPTIONAL] QBConaxPullEMMServiceSetNotifficationListener(...)
 * QBConaxPullEMMServiceStart(...)
 *
 * 2. Without using config server, with direct configuration:
 * QBConaxPullEMMServiceCreate(...)
 * QBConaxPullEMMServiceSetPullConfig(...)
 * [OPTIONAL] QBConaxPullEMMServiceSetRequestTokenFile(...)
 * [OPTIONAL] QBConaxPullEMMServiceSetSheduledPullAfterMinTime(...)
 * [OPTIONAL] QBConaxPullEMMServiceSetNotifficationListener(...)
 * QBConaxPullEMMServiceStart(...)
 *
 * Note: Conax documentation explicitly specify port 8093 for polling config, but
 * for consistency, we use existing EMMURL variable from /etc/vod/xml.conf, so we
 * have complete URL (also with "http://" prefix) and thus server port
 * in addition to server host. Port number MUST be specified in URL, otherwise
 * it will be default 80 for HTTP.
 */

/**
 * create a service for EMM pull
 *
 * @ param[in] smartCardMonitor monitor for watching for smart card events
 * @return constructed service
 */
QBConaxPullEMMService QBConaxPullEMMServiceCreate(QBSmartCardMonitor smartCardMonitor);

/**
 * set the URL of a server that will be contacted to obtain configuration
 *
 * @param[in] self this object
 * @param[in] url URL of the configuration server
 */
void QBConaxPullEMMServiceSetPullConfigServerUrl(QBConaxPullEMMService self, SvString url);

/**
 * Set file, where last pull configuration should be stored. If this function is not called,
 * last configuration will be hold only in RAM.
 *
 * @param[in] self this object
 * @param[in] filename path to file where configuration should be stored
 */
void QBConaxPullEMMServiceSetPullConfigFile(QBConaxPullEMMService self, SvString filename);

/**
 * Set file, where requestToken from last successful EMM pull (and push to smartcard) should
 * be stored. If this function is not called, requestToken will be hold only in RAM.
 *
 * @paran[in] self this object
 * @param[in] filename path to file where token should be stored
 */
void QBConaxPullEMMServiceSetRequestTokenFile(QBConaxPullEMMService self, SvString filename);

/**
 * Do not request configuration from server. Instead use direct configuration.
 *
 * @param[in] self this object
 * @param[in] baseUrl URL of server that should be contacted for EMMs
 * @param[in] recurrentPollTimeMillis time in milliseconds since midnight at which EMM polling should occur
 * @param[in] minTimeBetweenRequestsMillis minimal time in milliseconds between requests to the server
 */
void QBConaxPullEMMServiceSetPullConfig(QBConaxPullEMMService self,
                                        SvString baseUrl,
                                        long int recurrentPollTimeMillis, // time since midnight in miliseconds
                                        long int minTimeBetweenRequestsMillis /* milliseconds */);

/**
 * set the object that will handle notifications from EMM pull service
 *
 * @param[in] self this object
 * @param[in] listener object implementing QBConaxPullEMMNotificationListener
 */
void QBConaxPullEMMServiceSetNotificationListener(QBConaxPullEMMService self, SvObject listener);

/**
 * start the service
 *
 * @param[in] self this object
 * @param[in] scheduler SvScheduler that will be used to run fibers
 */
void QBConaxPullEMMServiceStart(QBConaxPullEMMService self, SvScheduler scheduler);

/**
 * stop the service
 *
 * @param[in] self this object
 */
void QBConaxPullEMMServiceStop(QBConaxPullEMMService self);

/**
 * enables or disables pulling EMMs from server each minTimeBetweenRequests instead of pulling once per day
 *
 * @param[in] self this object
 * @param[in] enabled decides if a feature should be on or off.
 *                    set to true if service should pull EMMs each
 *                    minTimeBetweenRequests and false if polling s
 *                    hould be done once per day
 */
void QBConaxPullEMMServiceSetSheduledPullAfterMinTime(QBConaxPullEMMService self, bool enabled);

/**
 * @}
 */

#endif

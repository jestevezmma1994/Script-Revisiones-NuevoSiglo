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
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef QB_SECURE_LOG_MANAGER_H_
#define QB_SECURE_LOG_MANAGER_H_

/**
 * @file QBSecureLogManager.h Secure Logger Manager class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvHashTable.h>

/**
* @defgroup  QBSecureLogManager Secure Logger Manager service
* @ingroup CubiTV_services
* @{
*
* This service is responsible for logging various events and sending them out to the
* remote HTTPS server. Each event contains its source (module name), event type and message.
* LogManager collects events and send them in bulk as so called packages. Packages are send out
* if they reach previously defined size or after timeout. Each package is compressed and might be
* additionally encrypted.
*
* Event type is defined as EventLevel.EventSource.EventAction where:
*   EventLevel   describes importance of message which might be set to: Error, Warning or Notice.
*   EventSource  describes additional informations related to the source of event.
*   EventAction  describes action upon witch an event occurred.
*
* Additionally each event that is logged might be filtered out based on its type or module name.
* It is possible to filter exact event types (such as EventLevel.EventSource.EventAction). Filtering
* also allows to use '*' sign instead of exact EventLevel, EventSource or EventAction to filter any
* event that match with pattern.
*
**/

/**
 * Typedef of function which creates custom header for SecureLog event.
 *
 * @param[in] self_     QBSecureLogLogic object
 * @return              created custom hello content
 **/
typedef SvString (*customHelloContentType)(SvObject self_);

/**
 * Typedef of function which creates custom header for SecureLog event.
 *
 * @param[in] self_     QBSecureLogLogic object
 * @param[in] eventType type of logged event
 * @return              created custom header
 **/
typedef SvString (*customHeaderType)(SvObject self_, SvString eventType);

/**
 * Secure Log Manager parameters for QBSecureLogManagerStart.
 **/
typedef struct QBSecureLogManagerParams_ {
    const char *serverName;         ///< address of remote log server
    const char *keyAES;             ///< key for optional AES encryption
    unsigned bufferSize;            ///< maximal size of log package
    SvString moduleFilters;         ///< module filters
    SvString eventFilters;          ///< event filters
    unsigned numOfRetries;          ///< number of retries for sending log
    unsigned numOfRetriesAtRestart; ///< number of retries for sending log when device is restarted
    size_t localBufferSize;         ///< maximal size of log buffer. when the buffer reaches its limit then the oldest packages will be dropped
    unsigned sendTimeout;           ///< number of seconds used by CURL as timeout for sending data
    SvHashTable eventsConfig;       ///< events configuration
} *QBSecureLogManagerParams;

/**
 * Start log manager.
 *
 * @param[in] params    parameters for Secure Log Manager
 * @return              true on success, false otherwise
 **/
bool QBSecureLogManagerStart(QBSecureLogManagerParams params);

/**
 * Stop log manager.
 **/
void QBSecureLogManagerStop(void);

/**
 * Return setting for given event
 *
 * @param[in] eventType  type of event
 * @param[in] key        name of setting
 * @return               found value on success, NULL otherwise
 */
SvValue QBSecureLogManagerGetEventConfig(SvString eventType, SvString key);

/**
 * Sends out massage to server and syslog.
 *
 * @param[in] logModuleName name of module
 * @param[in] eventType     type of event
 * @param[in] fmt           message format specification
 * @param[in] ...           arguments for format specification
 **/
void QBSecureLogEvent(const char* logModuleName, const char* eventType, const char* fmt, ...);

/**
 * Initialize secure logging manager module.
 *
 * @param[in] secureLogLogic QBSecureLogLogic object
 **/
void QBSecureLogManagerInit(SvObject secureLogLogic);

/**
 * Deinitialize secure logging manager module.
 **/
void QBSecureLogManagerDeinit(void);

/**
 * Register custom hello content function.
 *
 * @param[in] customHelloContent pointer to custom header function
 */
void QBSecureLogManagerSetCustomHelloContentFunction(customHelloContentType customHelloContent);

/**
 * Register custom header function.
 *
 * @param[in] customHeader pointer to custom header function
 */
void QBSecureLogManagerSetCustomHeaderFunction(customHeaderType customHeader);

/**
* @}
**/

#endif /* QB_SECURE_LOG_MANAGER_H_ */

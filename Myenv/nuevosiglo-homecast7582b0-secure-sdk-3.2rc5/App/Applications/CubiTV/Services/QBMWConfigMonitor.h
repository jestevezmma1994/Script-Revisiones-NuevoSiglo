/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_MW_CONFIG_MONITOR_H
#define QB_MW_CONFIG_MONITOR_H

/**
 * @file QBMWConfigMonitor.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief Cubiware MW config monitor.
 **/

#include <main_decl.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <stdbool.h>

/**
 * @defgroup QBMWConfigMonitor Cubiware MW Config Monitor
 * @ingroup CubiTV_services
 * @{
 *
 * This service tries to get some STB configuration settings from the
 * Cubiware MW. It sends request (with retries) and notifies it's listeners
 * sending them contents of the response.
 *
 * This service tries to connect to MW every @c requestsIntervalMs. If the
 * connection fails it enters a loop where it retries to connnect after
 * @c initialRetryIntervalMs, 2 * @c initialRetryIntervalMs, 4 *
 * @c initialRetryIntervalMs, 8 * @c initialRetryIntervalMs, etc, up to
 * @c maxRetryIntervalMs.
 *
 * Every time interval is multiplied by a random value between 0.5 and 1.5.
 **/

/**
 * Retries configuration.
 **/
typedef struct {
    size_t initialRetryIntervalMs;      ///< initial interval between retries in [ms]
    size_t maxRetryIntervalMs;          ///< maximum interval between retries in [ms]
    size_t requestsIntervalMs;          ///< time to wait to begin next request in [ms]
} QBMWConfigMonitorRetryPolicy;

/**
 * @brief QBMWConfigMonitor type
 */
typedef struct QBMWConfigMonitor_t *QBMWConfigMonitor;

/**
 * @brief QBMWConfigMonitorCreate
 * @param[in] appGlobals            AppGlobals handle
 * @param[in] retryPolicy           QBMWConfigMonitorRetryPolicy settings
 * @param[out] errorOut             error info
 * @return
 */
extern QBMWConfigMonitor
QBMWConfigMonitorCreate(AppGlobals appGlobals,
                        QBMWConfigMonitorRetryPolicy retryPolicy,
                        SvErrorInfo *errorOut);

/**
 * Start Cubi MW config monitor.
 *
 * Send request for configuration.
 *
 * @param[in] self      config monitor handle
 */
extern void
QBMWConfigMonitorStart(QBMWConfigMonitor self);

/**
 * Stop Cubi MW config monitor.
 *
 * @param[in] self      config monitor handle
 */
extern void
QBMWConfigMonitorStop(QBMWConfigMonitor self);

/**
 * Return config requests retry policy.
 *
 * @param[in] self          config monitor handle
 * @param[out] outPolicy    ponter to the policy struct that should be filled
 * @return                  true if the outPolicy was filled
 */
extern bool
QBMWConfigMonitorGetRetryPolicy(QBMWConfigMonitor self, QBMWConfigMonitorRetryPolicy* outPolicy);

/**
 * Add config response listener.
 * @param[in] self      config monitor handle
 * @param[in] listener  listener object
 */
extern void
QBMWConfigMonitorAddListener(QBMWConfigMonitor self, SvGenericObject listener);

/**
 * Remove config response listener.
 * @param[in] self      config monitor handle
 * @param[in] listener  listener object
 */
extern void
QBMWConfigMonitorRemoveListener(QBMWConfigMonitor self, SvGenericObject listener);

/**
 * Get runtime type identification object representing QB MW config listener interface.
 * @return
 */
SvInterface
QBMWConfigMonitorListener_getInterface(void);

/**
 * Inform if the Cubi MW responded to the config request.
 * @param[in] self      config monitor handle
 * @return              true if the MW answered and answer was correct
 */
extern bool
QBMWConfigMonitorHasConfiguration(QBMWConfigMonitor self);

/**
 * QBMWConfigMonitorListener interface
 */
typedef struct QBMWConfigMonitorListener_ {
    /**
     * Notify that middleware send correct answer
     *
     * @param[in] self_     handler to an object implementing
     *                      @ref QBMWConfigMonitorListener
     * @param[in] results   data received from middleware
     */
    void (*gotResponse) (SvGenericObject self_, SvHashTable results);

    /**
     * Notify that middleware send invalid answer or request state got exception
     * @param[in] self_     handler to an object implementing
     *                      @ref QBMWConfigMonitorListener
     */
    void (*noResponse) (SvGenericObject self_);
} *QBMWConfigMonitorListener;

/**
 * @}
 **/

#endif // QB_MW_CONFIG_MONITOR_H

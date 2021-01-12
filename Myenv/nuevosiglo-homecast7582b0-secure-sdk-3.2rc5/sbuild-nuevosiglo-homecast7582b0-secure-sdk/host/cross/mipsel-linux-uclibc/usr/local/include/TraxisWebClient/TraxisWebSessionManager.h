/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef TRAXIS_WEB_SESSION_MANAGER_H_
#define TRAXIS_WEB_SESSION_MANAGER_H_

/**
 * @file TraxisWebSessionManager.h
 * @brief Traxis.Web session manager class
 **/

#include <stddef.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvHTTPClient/SvSSLParams.h>

#include <TraxisWebClient/TraxisWebAuthStatus.h>
#include <TraxisWebClient/TraxisWebAuthLevel.h>
#include <QBAppKit/QBCacheController.h>


/**
 * @defgroup TraxisWebSessionManager Traxis.Web session manager class
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web session manager class.
 * @class TraxisWebSessionManager
 * @extends SvObject
 **/
typedef struct TraxisWebSessionManager_ *TraxisWebSessionManager;

/**
 * Traxis.Web session state.
 **/
typedef enum {
    /// unknown state of the session
    TraxisWebSessionState_unknown = -1,
    /// freshly created session, manager not yet started
    TraxisWebSessionState_initial = 0,
    /// initial handshake (gathering API version, authenticating etc.)
    TraxisWebSessionState_handshake,
    /// ready to process requests but without cpe id
    TraxisWebSessionState_anonymous,
    /// ready to process requests
    TraxisWebSessionState_active,
    /// session suspended
    TraxisWebSessionState_stopped
} TraxisWebSessionState;

struct TraxisWebServerVersion {
    SvString server;
    struct {
        unsigned int major, minor;
        SvString str;
    } API;
};

/**
 * Get runtime type identification object representing
 * type of Traxis.Web session manager class.
 *
 * @return Traxis.Web session manager class
 **/
extern SvType
TraxisWebSessionManager_getType(void);

/**
 * Create Traxis.Web session manager.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] serverURL Traxis.Web server URL
 * @param[out] errorOut error info
 * @return              created session manager, @c NULL in case of error
 **/
extern TraxisWebSessionManager
TraxisWebSessionManagerCreate(SvURL serverURL,
                              SvErrorInfo *errorOut);

/**
 * Set custom authentication plugin.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] plugin    handle to an object implementing @ref TraxisWebAuthPlugin
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerSetAuthPlugin(TraxisWebSessionManager self,
                                     SvObject plugin,
                                     SvErrorInfo *errorOut);

/**
 * Get Traxis.Web authentication status.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @return              authentication status
 **/
extern TraxisWebAuthStatus
TraxisWebSessionManagerGetAuthStatus(TraxisWebSessionManager self);

/**
 * Get Traxis.Web session state.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @return              session state
 **/
extern TraxisWebSessionState
TraxisWebSessionManagerGetState(TraxisWebSessionManager self);

/**
 * Get Traxis.Web server URL.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @return              server URL, @c NULL if unavailable
 **/
extern SvURL
TraxisWebSessionManagerGetServerURL(TraxisWebSessionManager self);

/**
 * Set SSL parameters for HTTPS communication with Traxis.Web server.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] sslParams SSL parameters
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerSetSSLParams(TraxisWebSessionManager self,
                                    SvSSLParams sslParams,
                                    SvErrorInfo *errorOut);

/**
 * Get Traxis.Web server version.
 *
 * @note Version information is available after handshake.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[out] errorOut error info
 * @return              server version information
 **/
extern const struct TraxisWebServerVersion *
TraxisWebSessionManagerGetServerVersion(TraxisWebSessionManager self,
                                        SvErrorInfo *errorOut);

/**
 * Set client identification parameters.
 *
 * This method sets all the information that is required for STB
 * to identify itself to the Traxis.Web server.
 *
 * For servers implementing Traxis.Web API version 1 STB will use
 * device (CPE) identifier and profile name to get its unique identity.
 *
 * For servers implementing Traxis.Web API version 2 STB will
 * pass all known parameters in each request.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self       Traxis.Web session manager handle
 * @param[in] CPEID      device identifier, required for Traxis.Web version 1
 * @param[in] customerID customer identifier, required for Traxis.Web version 2
 *                       (ignored for version 1)
 * @param[in] profileName name of the profile, optional
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerSetIdentification(TraxisWebSessionManager self,
                                         SvString CPEID,
                                         SvString customerID,
                                         SvString profileName,
                                         SvErrorInfo *errorOut);

/**
 * Set required branding for device identity.
 *
 * This method extends handshake process with a check if branding
 * name associated with device identity matches @a branding; if not,
 * identity is updated.
 *
 * @note Identities are present only in Traxis.Web API version 1. This method
 * has no effect when server implements other version.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] branding  branding for device (CPE) identity
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerSetIdentityBranding(TraxisWebSessionManager self,
                                           SvString branding,
                                           SvErrorInfo *errorOut);

/**
 * Get device identifier.
 *
 * @note 'CPE' in Traxis.Web terminology means 'Customer Premises Equipment'.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @return              device identifier, @c NULL if unavailable
 **/
extern SvString
TraxisWebSessionManagerGetCPEID(TraxisWebSessionManager self);

/**
 * Get profile name.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @return              profile name, @c NULL if unavailable
 **/
extern SvString
TraxisWebSessionManagerGetProfileName(TraxisWebSessionManager self);

/**
 * Get customer identifier.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @return              customer identifier, @c NULL if unavailable
 **/
extern SvString
TraxisWebSessionManagerGetCustomerID(TraxisWebSessionManager self);

/**
 * Set requested content language.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] language  language name
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerSetLanguage(TraxisWebSessionManager self,
                                   SvString language,
                                   SvErrorInfo *errorOut);

/**
 * Get current language.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @return              current language, @c NULL if unavailable
 **/
extern SvString
TraxisWebSessionManagerGetLanguage(TraxisWebSessionManager self);

/**
 * Set value of custom HTTP query parameter to add to all requests.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] paramName name of the parameter
 * @param[in] paramValue new value of the parameter, @c NULL to remove
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerSetQueryParam(TraxisWebSessionManager self,
                                     SvString paramName,
                                     SvString paramValue,
                                     SvErrorInfo *errorOut);

/**
 * Add session state listener.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] listener  handle to an object implementing @ref TraxisWebSessionStateListener
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerAddListener(TraxisWebSessionManager self,
                                   SvObject listener,
                                   SvErrorInfo *errorOut);

/**
 * Remove session state listener.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] listener  handle to a previously registered state listener
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerRemoveListener(TraxisWebSessionManager self,
                                      SvObject listener,
                                      SvErrorInfo *errorOut);

/**
 * Start session.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] scheduler scheduler to be used for entire Traxis.Web session
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerStart(TraxisWebSessionManager self,
                             SvScheduler scheduler,
                             SvErrorInfo *errorOut);

/**
 * Stop session.
 *
 * @note This method will cancel all ongoing requests.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerStop(TraxisWebSessionManager self,
                            SvErrorInfo *errorOut);

/**
 * Notify session manager that authentication status might have changed.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerCheckAuthStatus(TraxisWebSessionManager self,
                                       SvErrorInfo *errorOut);

/**
 * FIXME: documentation!
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self      Traxis.Web session manager handle
 * @param[in] serverURL Traxis.Web server URL
 * @param[out] errorOut error info
 **/
extern void
TraxisWebSessionManagerSetServerURL(TraxisWebSessionManager self,
                                    SvURL serverURL,
                                    SvErrorInfo *errorOut);

extern void
TraxisWebSessionManagerSetAnonymousMode(TraxisWebSessionManager self,
                                        bool anonymousMode);

extern void
TraxisWebSessionManagerSetUsePlaybackSessionManager(TraxisWebSessionManager self,
                                                    bool usePlaybackSessionManager);

extern bool
TraxisWebSessionManagerGetUsePlaybackSessionManager(TraxisWebSessionManager self);

extern void
TraxisWebSessionManagerSetUsePosterServerInterface(TraxisWebSessionManager self, bool usePosterServerInterface);

extern bool
TraxisWebSessionManagerGetUsePosterServerInterface(TraxisWebSessionManager self);

extern void
TraxisWebSessionManagerSetUseWildcardSearch(TraxisWebSessionManager self, bool useWildcardSearch);

extern bool
TraxisWebSessionManagerGetUseWildcardSearch(TraxisWebSessionManager self);

/**
 * Set common cache cache controller.
 *
 * Cache controller is used as assets cache.
 *
 * @memberof TraxisWebSessionManager
 *
 * @param[in] self            Traxis.Web session manager handle
 * @param[in] cacheController cache controller handle
 * @param[out] errorOut       error info
 **/
extern void
TraxisWebSessionManagerSetCacheController(TraxisWebSessionManager self,
                                          QBCacheController cacheController,
                                          SvErrorInfo *errorOut);

/**
 * @}
 **/


/**
 * @defgroup TraxisWebSessionStateListener Traxis.Web session state listener interface
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web session state listener interface.
 **/
typedef const struct TraxisWebSessionStateListener_ {
    /**
     * Notify that session state have changed.
     *
     * @param[in] self      session state listener handle
     * @param[in] state     new session state
     **/
    void (*stateChanged)(SvObject self_,
                         TraxisWebSessionState state);

    /**
     * Notify that session language have changed.
     *
     * @param[in] self      session state listener handle
     * @param[in] language  new session language
     **/
    void (*languageChanged)(SvObject self_,
                            SvString language);

    void (*authLevelChanged)(SvObject self_,
                             TraxisWebAuthLevel level);
} *TraxisWebSessionStateListener;

/**
 * Get runtime type identification object representing
 * type of Traxis.Web session state listener interface.
 * @return Traxis Web Session State Listener interface handle
 **/
extern SvInterface
TraxisWebSessionStateListener_getInterface(void);

/**
 * @}
 **/


#endif

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

#ifndef TRAXIS_WEB_NPVR_PLUGIN_H_
#define TRAXIS_WEB_NPVR_PLUGIN_H_

/**
 * @file TraxisWebNPvrPlugin.h
 * @brief Traxis.Web NPVR plugin class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <TraxisWebClient/TraxisWebSessionManager.h>
#include <NPvr/QBnPVRTypes.h>

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * @defgroup TraxisWebNPvrPlugin Traxis.Web NPVR plugin class
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web NPVR plugin class.
 * @class TraxisWebNPvrPlugin
 * @extends SvObject
 **/
typedef struct TraxisWebNPVRProvider_s TraxisWebNPVRProvider;

/**
 * Configuration of the Traxis.Web NPVR plugin.
 **/
typedef struct TraxisWebNPvrProviderInitConfig_s {
    /// sync recording list with the server - period in seconds, use @c 0 for default
    unsigned int recordingsRefreshPeriod;
    /// sync recording list with the server - period in seconds, used when previous sync failed, use @c 0 for default
    unsigned int recordingsRefreshPeriodFailedRetry;

    /// sync quota status with the server - period in seconds, use @c 0 for default
    unsigned int quotaRefreshPeriod;
    /// sync quota status with the server - period in seconds, used when previous sync failed, use @c 0 for default
    unsigned int quotaRefreshPeriodFailedRetry;
    /// @c true iff a content is playable while its recording is still ongoing (else: only playable when state is "completed")
    bool  playableWhenRecording;
} TraxisWebNPvrProviderInitConfig;

/**
 * Get runtime type identification object
 * representing Traxis.Web NPVR plugin class.
 *
 * @return Traxis.Web NPVR plugin class
 **/
extern SvType
TraxisWebNPVRProvider_getType(void);

/**
 * Initialize Traxis.Web NPVR plugin instance.
 *
 * @memberof TraxisWebNPvrPlugin
 *
 * @param[in] self        Traxis.Web NPVR plugin handle
 * @param[in] sessionManager Traxis.Web session manager handle
 * @param[in] initConfig  configuration of the NPVR plugin
 * @param[out] errorOut   error info
 * @return                @a self or @c NULL in case of error
 **/
extern TraxisWebNPVRProvider*
TraxisWebNPVRProviderInit(TraxisWebNPVRProvider* self,
                       TraxisWebSessionManager sessionManager,
                       const TraxisWebNPvrProviderInitConfig* initConfig,
                       SvErrorInfo *errorOut);

/**
 * Pass stats configuration.
 *
 * @memberof TraxisWebNPvrPlugin
 *
 * @param[in] self        Traxis.Web NPVR plugin handle
 * @param[in] config      structure returned with every "statsChanged" callback
 **/
extern void
TraxisWebNPVRProviderSetStatsConfig(TraxisWebNPVRProvider* self,
                                 const NPvrStatsConfig* config);

/**
 * @}
 **/


#endif

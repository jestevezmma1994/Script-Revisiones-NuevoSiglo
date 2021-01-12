/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011 Cubiware Sp. z o.o. All rights reserved.
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

/* App/Libraries/QBContentManager/Innov8onProviderParams.h */

#ifndef INNOV8ON_PROVIDER_PARAMS_H_
#define INNOV8ON_PROVIDER_PARAMS_H_

/**
 * @file Innov8onProviderParams.h
 * @brief Innov8on provider parameters class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup Innov8onProviderParams Innov8on provider parameters class
 * @ingroup Innov8on
 * @{
 **/

#include <stdlib.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>

/**
 * Innov8on provider parameters class.
 **/
typedef struct Innov8onProviderParams_ *Innov8onProviderParams;


/**
 * Get runtime type identification object representing
 * type of Innov8on provider parameters class.
 * @return Innov8on provider params runtime type identification object
 **/
SvType
Innov8onProviderParams_getType(void);

/**
 * Create Innov8on provider parameters object.
 *
 * @param[in] server    Innov8on XML-RPC server data
 * @param[in] deviceID  device ID of the box for Innov8on MW
 * @param[in] minRefreshPeriod minimal delay in seconds between consecutive
 *                      calls to MW for refreshing single category
 * @param[in] rangeSize number of categories/assets that are downloaded with one request
 * @param[in] categoryCacheSize number of categories that can be retained in memory for future reuse
 * @param[in] assetCacheSize number of assets that can be retained in memory for future reuse. NOTE the number given is not strictly enforced. Actually enforced number of cached assets can be as high as assetCacheSize + rangeSize.
 * @param[in] cacheTimeoutSec number of seconds till cached entry is thrown away and will need to be re-fetched when needed
 * @param[in] login     MW user login
 * @param[in] password  MW user password
 * @param[in] requestPlugin object implementing Innov8onProviderRequestPlugin
 * @param[in] enableSlaveStaticCategories @c true to (if possible) add static categories such as Search or AllVisible
 * @param[out] errorOut error info
 * @return              created object, @c NULL in case of error
 **/
Innov8onProviderParams
Innov8onProviderParamsCreate(SvXMLRPCServerInfo server,
                             SvString deviceID,
                             unsigned int minRefreshPeriod,
                             size_t rangeSize,
                             size_t categoryCacheSize,
                             size_t assetCacheSize,
                             int cacheTimeoutSec,
                             SvString login,
                             SvString password,
                             SvObject requestPlugin,
                             bool enableSlaveStaticCategories,
                             SvErrorInfo *errorOut);

/**
 * Setter, overwrites password in params.
 *
 * @param[in] self  provider params handle
 * @param[in] login new login
 *
 **/
void
Innov8onProviderParamsSetLogin(Innov8onProviderParams self, SvString login);

/**
 * Setter, overwrites password in params.
 * @param[in] self      provider params handle
 * @param[in] password  new password
 **/
void
Innov8onProviderParamsSetPassword(Innov8onProviderParams self, SvString password);

/**
 * Setter, overwrites request plugin in params.
 * @param[in] self          provider params handle
 * @param[in] requestPlugin new request plugin
 **/
void
Innov8onProviderParamsSetRequestPlugin(Innov8onProviderParams self, SvObject requestPlugin);


/**
 * @}
 **/

#endif

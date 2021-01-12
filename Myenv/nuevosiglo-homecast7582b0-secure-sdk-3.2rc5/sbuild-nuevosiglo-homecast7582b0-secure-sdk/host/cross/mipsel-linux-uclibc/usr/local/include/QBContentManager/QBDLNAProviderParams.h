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

#ifndef QB_DLNA_PROVIDER_PARAMS_H_
#define QB_DLNA_PROVIDER_PARAMS_H_

/**
 * @file QBDLNAProviderParams.h
 * @brief DLNA VoD provider parameters class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * @defgroup QBDLNAProviderParams DLNA VoD provider parameters class
 * @ingroup QBDLNA
 * @{
 **/

/**
 * DLNA VoD provider parameters class.
 * @class QBDLNAProviderParams
 * @extends SvObject
 **/
typedef struct QBDLNAProviderParams_ *QBDLNAProviderParams;


/**
 * Get runtime type identification object representing DLNA VoD provider parameters class.
 * @return DLNA VoD provider parameters class
 **/
extern SvType
QBDLNAProviderParams_getType(void);

/**
 * Create DLNA VoD provider parameters object.
 *
 * @param[in] minRefreshPeriod minimal delay in seconds between consecutive
 *                      calls to MW for refreshing single category
 * @param[in] rangeSize number of categories/assets that are downloaded with one request
 * @param[in] categoryCacheSize number of categories that can be retained in memory for future reuse
 * @param[in] assetCacheSize number of assets that can be retained in memory for future reuse. NOTE the number given is not strictly enforced. Actually enforced number of cached assets can be as high as assetCacheSize + rangeSize.
 * @param[in] cacheTimeoutSec number of seconds till cached entry is thrown away and will need to be re-fetched when needed
 * @param[in] sortDeviceList true if devices friendly names should be sorted alphabetically
 * @param[out] errorOut error info
 * @return              created object, @c NULL in case of error
 **/
QBDLNAProviderParams
QBDLNAProviderParamsCreate(unsigned int minRefreshPeriod,
                           size_t rangeSize,
                           size_t categoryCacheSize,
                           size_t assetCacheSize,
                           int cacheTimeoutSec,
                           bool sortDeviceList,
                           SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_MW_CLIENT_CONTENT_MANAGER_H_
#define QB_MW_CLIENT_CONTENT_MANAGER_H_

/**
 * @file QBMWClientContentManager.h
 * @brief QBMWClient content manager
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 **/

/**
 * @defgroup QBMWClientContentManager
 * @ingroup QBMWClient
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvURL.h>
#include <SvHTTPClient/SvOAuthParams.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <sys/types.h>


/**
 * QBMWClient content manager parameters.
 **/
typedef struct QBMWClientContentManagerParams_s {
    /// parent class
    struct SvObject_ super_;
    /// content server URL
    SvURL baseUrl;
    /// images server URL
    SvURL imagesBaseUrl;
    /// device ID
    SvString deviceId;
    /// customer ID
    SvString customerId;
    /// country code formatted according to ISO 3166-1 alpha-3 standard (the country code where the event occurred)
    SvString countryCode;
    /// vod offer country code used to filtering vod offer (different format than previous countryCode)
    SvString vodCountryCode;
    /// path to directory containing certificates used by content server
    SvString certsPath;
    /// content request range size
    ssize_t rangeSize;
    /// content range timeout (used to invalidate old content ranges)
    SvTime rangeTimeout;
    /// content range margin (used to spread content range requests in time)
    SvTime rangeMargin;
    /// oauth params
    struct {
        SvOAuthMethod method;
        SvString consumerKey;
        SvString consumerSecret;
    } oauthSettings;
    /// path to content mapper's map file
    SvString mapperConf;
    /// language code
    SvString langCode;
} *QBMWClientContentManagerParams;

/**
 * Get runtime type identification object representing
 * QBMWClient content manager parameters.
 * @return QBMWClient content manager parameters object
 **/
SvType
QBMWClientContentManagerParams_getType(void);

/**
 * QBMWClient content manager class.
 *
 * This class is responsible for providing content metadata
 * through creating and handling appropriate content providers.
 *
 * @class QBMWClientContentManager
 * @extends SvObject
 **/
typedef struct QBMWClientContentManager_ *QBMWClientContentManager;

/**
* Get runtime type identification object representing QBMWClientContentManager class.
*
* @return QBMWClient content manager class
**/
SvType QBMWClientContentManager_getType(void);

/**
 * A function initializing content manager.
 *
 * @param[in]  self      pointer to content manager
 * @param[in]  params    content manager parameters
 * @param[out] errorOut  error info
 * @return               @c 0 on success, @c -1 in case of error
 **/
int
QBMWClientContentManagerInit(QBMWClientContentManager self,
                             QBMWClientContentManagerParams params,
                             SvErrorInfo *errorOut);

/**
 * Set content manager query language.
 *
 * @param[in]  self      pointer to content manager
 * @param[in]  langCode  language code
 * @param[out] errorOut  error info
 *
 **/
void
QBMWClientContentManagerSetLanguage(QBMWClientContentManager self,
                                    SvString langCode,
                                    SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QB_MW_CLIENT_CONTENT_MANAGER_H_

/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SvFoundation_SvURL_h
#define SvFoundation_SvURL_h

/**
 * @file SvURL.h URL class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvFoundationErrorDomain.h>
#include <SvFoundation/SvURI.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvURL URL class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * URL class.
 * @class SvURL
 * @extends SvURI
 **/
typedef struct SvURI_ *SvURL;


/**
 * Get runtime type identification object representing SvURL class.
 *
 * @return URL class
 **/
extern SvType
SvURL_getType(void);

/**
 * Create an absolute URL.
 *
 * @memberof SvURL
 *
 * @param[in] url absolute URL string
 * @param[out] errorOut error info
 * @return created URL or @c NULL in case of error
 **/
extern SvURL
SvURLCreateWithString(SvString url,
                      SvErrorInfo *errorOut);

/**
 * Create an absolute URL from a relative URL and other absolute URL.
 *
 * @memberof SvURL
 *
 * @param[in] url relative URL string
 * @param[in] base base URL handle
 * @param[out] errorOut error info
 * @return created URL or @c NULL in case of error
 **/
extern SvURL
SvURLCreateWithStringRelativeToURL(SvString url,
                                   SvURL base,
                                   SvErrorInfo *errorOut);

/**
 * Create an URL from URI object.
 *
 * @memberof SvURL
 *
 * @param[in] uri handle to an absolute URI
 * @param[out] errorOut error info
 * @return created URL or @c NULL in case of error
 **/
extern SvURL
SvURLCreateWithURI(SvURI uri,
                   SvErrorInfo *errorOut);

/**
 * Get the URL string.
 *
 * @memberof SvURL
 *
 * @param[in] self URL handle
 * @return URL as string
 **/
extern SvString
SvURLString(SvURL self);

/**
 * Check if URL is absolute.
 *
 * @memberof SvURL
 *
 * @param[in] self URL handle
 * @return @c true if URL is absolute, @c false otherwise
 **/
extern bool
SvURLIsAbsolute(SvURL self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

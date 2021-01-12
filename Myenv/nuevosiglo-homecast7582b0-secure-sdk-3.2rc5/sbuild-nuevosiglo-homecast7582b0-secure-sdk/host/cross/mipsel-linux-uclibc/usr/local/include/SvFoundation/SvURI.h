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

#ifndef SvFoundation_SvURI_h
#define SvFoundation_SvURI_h

/**
 * @file SvURI.h URI class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvFoundationErrorDomain.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvURI URI class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * URI class.
 * @class SvURI
 * @extends SvObject
 **/
typedef struct SvURI_ *SvURI;


/**
 * Get runtime type identification object representing SvURI class.
 *
 * @return URI class
 **/
extern SvType
SvURI_getType(void);

/**
 * Create an URI (not URL-encoded) from raw (not URL-encoded) C string.
 *
 * @memberof SvURI
 *
 * @param[in] uri URI string
 * @param[out] errorOut error info
 * @return created URI or @c NULL in case of error
 **/
extern SvURI
SvURICreate(const char *uri,
            SvErrorInfo *errorOut);

/**
 * Create an URI (and encode if necessary) from raw (not URL-encoded) C string.
 *
 * @memberof SvURI
 *
 * @param[in] uri URI string
 * @param[out] errorOut error info
 * @return created URI or @c NULL in case of error
 **/
extern SvURI
SvURICreateEncoded(const char *uri,
                   SvErrorInfo *errorOut);

/**
 * Create an URI from (not URL-encoded) string.
 *
 * @memberof SvURI
 *
 * @param[in] uri URI string
 * @param[out] errorOut error info
 * @return created URI or @c NULL in case of error
 **/
extern SvURI
SvURICreateWithString(SvString uri,
                      SvErrorInfo *errorOut);

/**
 * Create an URI from a base URI and relative or absolute path string.
 *
 * @memberof SvURI
 *
 * @param[in] base base URI
 * @param[in] path path part of the URI to create
 * @param[out] errorOut error info
 * @return created URI or @c NULL in case of error
 **/
extern SvURI
SvURICreateWithBaseURIAndPath(SvURI base,
                              SvString path,
                              SvErrorInfo *errorOut);

/**
 * Create an URI from parts.
 *
 * @memberof SvURI
 *
 * This method creates URI from separate parts.
 * Only the @a path part is mandatory, @a authority and @a query
 * parts cannot be used without specifying @a scheme.
 *
 * @param[in] scheme URI scheme
 * @param[in] authority URI authority, requires @a scheme
 * @param[in] path URI path, mandatory
 * @param[in] query URI query, requires @a scheme
 * @param[in] fragment URI fragment
 * @param[out] errorOut error info
 * @return created URI or @c NULL in case of error
 **/
extern SvURI
SvURIConstruct(const char *scheme,
               const char *authority,
               const char *path,
               const char *query,
               const char *fragment,
               SvErrorInfo *errorOut);

/**
 * Check if URI is an URL.
 *
 * @memberof SvURI
 *
 * @param[in] self URI handle
 * @return @c true if URI is a Uniform Resource Locator, @c false otherwise
 **/
extern bool
SvURIIsURL(SvURI self);

/**
 * Check if URI is absolute.
 *
 * @memberof SvURI
 *
 * @param[in] self URI handle
 * @return @c true if URI is absolute, @c false otherwise
 **/
extern bool
SvURIIsAbsolute(SvURI self);

/**
 * Get the full URI string.
 *
 * @memberof SvURI
 *
 * @param[in] self URI handle
 * @return URI as string
 **/
extern SvString
SvURIString(SvURI self);

/**
 * Get the URI scheme string.
 *
 * @memberof SvURI
 *
 * @param[in] self URI handle
 * @return URI scheme part as string
 **/
extern SvString
SvURIScheme(SvURI self);

/**
 * Get the authority part of the URI.
 *
 * @memberof SvURI
 *
 * @param[in] self URI handle
 * @return URI authority part as string
 **/
extern SvString
SvURIAuthority(SvURI self);

/**
 * Get the path part of the URI.
 *
 * @memberof SvURI
 *
 * @param[in] self URI handle
 * @return URI path part as string
 **/
extern SvString
SvURIPath(SvURI self);

/**
 * Get the query part of the URI.
 *
 * @memberof SvURI
 *
 * @param[in] self URI handle
 * @return URI query part as string
 **/
extern SvString
SvURIQuery(SvURI self);

/**
 * Get the fragment part of the URI.
 *
 * @memberof SvURI
 *
 * @param[in] self URI handle
 * @return URI fragment part as string
 **/
extern SvString
SvURIFragment(SvURI self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

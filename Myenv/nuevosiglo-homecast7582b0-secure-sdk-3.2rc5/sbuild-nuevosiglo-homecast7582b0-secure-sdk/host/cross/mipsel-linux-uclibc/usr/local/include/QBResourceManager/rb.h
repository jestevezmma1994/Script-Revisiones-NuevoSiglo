/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_RB_SERVICE_H_
#define SV_RB_SERVICE_H_

/**
 * @file rb.h Resource manager service compatibility API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <stddef.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBResourceManager/SvRBObject.h>
#include <QBResourceManager/QBResourceManager.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvRBService Resource manager service compatibility API
 * @ingroup QBResourceManagerLibrary
 * @{
 *
 * This is an old API for the @link QBResourceManager @endlink maintained for backwards compatibility, it will be replaced with modern one in the future.
 **/

/**
 * Register new object in the resource manager service.
 *
 * @param[in] self      resource manager service handle
 * @param[in] obj       handle to a resource manager object to be added
 * @param[in] policy    object removal policy
 * @return              unique identifier assigned for @a obj
 **/
extern SvRID svRBAddItem(QBResourceManager self, SvRBObject obj, SvRBPolicy policy);

/**
 * Get an object from the resource manager by its unique identifier.
 *
 * @param[in] self      resource manager service handle
 * @param[in] rid       object's unique identifier
 * @return              handle to a resource manager object with @a rid
 *                      or @c NULL if such object does not exist
 **/
extern SvRBObject svRBGetItem(QBResourceManager self, SvRID rid);

/**
 * Find an object in the resource manager representing a resource
 * originating from given URI.
 *
 * @param[in] self      resource manager service handle
 * @param[in] URI       resource URI
 * @return              handle to a resource manager object
 *                      or @c NULL if such object does not exist
 **/
extern SvRBObject svRBFindItem(QBResourceManager self, SvURI URI);

/**
 * Find an object in the resource manager representing a resource
 * originating from given URI.
 *
 * @note This is a convenience wrapper for svRBFindItem().
 *
 * @param[in] self      resource manager service handle
 * @param[in] URI       resource URI
 * @return              handle to a resource manager object
 *                      or @c NULL if such object does not exist
 **/
extern SvRBObject svRBFindItemByURI(QBResourceManager self, const char *URI);

/**
 * Request loading and decoding of a CAGE resource represented by
 * a resource manager object.
 *
 * This functions requests that the CAGE resource (bitmap or font),
 * represented by an object registered in the resource manager service,
 * should be downloaded (if it is in a remote location) and prepared
 * to be used in GUI application.
 *
 * Sometimes the requested resource is available immediately; in such
 * case, it is returned in @a res parameter. Otherwise, when the loading
 * process is finished, all interested listeners are notified.
 *
 * @param[in] self      resource manager service handle
 * @param[in] rid       resource ID
 * @param[in] listener  handle to an object that will be notified when loading
 *                      is finished
 * @param[out] obj      handle to a resource manager object with @a rid
 * @param[out] res      handle to a CAGE resource (SvBitmap or SvFont)
 *                      or @c NULL if it is not available immediately
 * @return              @c 0 in case of error, otherwise any other value
 **/
extern int svRBLookupItem(QBResourceManager self, SvRID rid, SvObject listener,
                          SvRBObject *obj, void **res);

/**
 * Unregister object from the queue waiting for resource to be loaded.
 *
 * @param[in] self      resource manager service handle
 * @param[in] rid       resource ID
 * @param[in] listener  handle to an object that requested resource using svRBLookupItem()
 * @return              @c 0 in case of error, otherwise any other value
 **/
extern int svRBCancelLookup(QBResourceManager self, SvRID rid, SvObject listener);

/**
 * Change removal policy of an object that is already present in the resource manager.
 *
 * @param[in] self      resource manager service handle
 * @param[in] obj       handle to a resource manager object
 * @param[in] policy    new object removal policy, can only be more restrictive than current one
 * @return              @c 0 in case of error, otherwise any other value
 **/
extern int svRBSetItemPolicy(QBResourceManager self, SvRBObject obj, SvRBPolicy policy);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

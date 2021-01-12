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

#ifndef SV_RB_OBJECT_H_
#define SV_RB_OBJECT_H_

/**
 * @file SvRBObject.h
 * @brief Resource manager object class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURI.h>
#include <SvFoundation/SvURL.h>
#include <SvHTTPClient/SvHTTPFileCache.h>
#include <QBAppKit/QBThreadPool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvRBObject Resource manager object class
 * @ingroup QBResourceManagerLibrary
 * @{
 *
 * An abstract class representing a resource managed by the resource manager.
 **/


/// Unique resource identifier type.
typedef unsigned int SvRID;

/// Invalid identifier value.
#define SV_RID_INVALID  ((SvRID) 0)

/**
 * Flags specifying how resource manager service should treat
 * a registered object when it is no longer used.
 **/
typedef enum {
    SvRBPolicy_auto = 0, ///< resource manager will decide on its own
    SvRBPolicy_static    ///< item never removed from memory once loaded
} SvRBPolicy;


/**
 * Resource manager object class.
 * @class SvRBObject SvRBObject.h <QBResourceManager/SvRBObject.h>
 * @extends SvObject
 */
typedef struct SvRBObject_ *SvRBObject;


/**
 * Get runtime type identification object representing type
 * of resource manager object class.
 *
 * @relates SvRBObject
 *
 * @return resource manager object class
 **/
extern SvType SvRBObject_getType(void);

/**
 * Get unique identifier of a resource manager object.
 *
 * This method returns unique resource identifier of a resource manager object.
 *
 * @note After creation, object's identifier can have the value
 * of @c SV_RID_INVALID - it will change when the object is added to the
 * resource manager.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @return              unique resource ID
 **/
extern SvRID SvRBObjectGetID(SvRBObject self);

/**
 * Set unique identifier of a resource manager object.
 *
 * This method sets unique resource identifier of an object. It is permitted
 * to do so only if the identifier had not been set before.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @param[in] rid       unique resource ID
 * @param[out] errorOut error info
 **/
extern void SvRBObjectSetID(SvRBObject self,
                            SvRID rid,
                            SvErrorInfo *errorOut);

/**
 * Get unload policy of the resource manager object.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @return              unload policy
 **/
extern SvRBPolicy SvRBObjectGetPolicy(SvRBObject self);

/**
 * Get size of the resource represented by a resource manager object.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @return              approximate amount of memory used by the resource
 *                      (in bytes), @c 0 if resource is not loaded yey
 **/
extern size_t SvRBObjectGetSize(SvRBObject self);

/**
 * Return URI where the resource is kept.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @return              resource URI, @c NULL if not available
 **/
extern SvURI SvRBObjectGetURI(SvRBObject self);

/**
 * Check if the resource represented by a resource manager object
 * has been loaded.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @return              @c true if resource is available now, @c false otherwise
 **/
extern bool SvRBObjectResourceIsLoaded(SvRBObject self);

/**
 * Check if loading of the resource represented by a resource manager object
 * has been cancelled.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @return              @c true if resource loading has been cancelled, @c false otherwise
 **/
extern bool SvRBObjectResourceIsCancelled(SvRBObject self);

/**
 * Check if resource represented by an object is used.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @return              @c true if resource is used, @c false if not
 **/
extern bool SvRBObjectIsResourceInUse(SvRBObject self);

/**
 * Load the resource from a file in local file system.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @param[in] localPath path to the local resource file
 * @param[in] cacheEntry handle to the cache entry representing the source
 *                      file, @c NULL if this file is not in HTTP cache
 * @param[in] threadPool handle to a thread pool that can be used to load
 *                      resources in the background (optional: can be @c NULL)
 * @return              loaded resource or @c NULL in case of error
 **/
extern void *SvRBObjectLoadResource(SvRBObject self,
                                    const char *localPath,
                                    SvHTTPFileCacheEntry cacheEntry,
                                    QBThreadPool threadPool);

/**
 * Cancel loading the resource.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 **/
extern void SvRBObjectCancelLoadingResource(SvRBObject self);

/**
 * Return the resource represented by a resource manager object.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object
 * @return              handle to the resource, @c NULL if it is not available
 **/
extern void *SvRBObjectGetResource(SvRBObject self);

/**
 * Free the resource represented by a resource manager object.
 *
 * @memberof SvRBObject
 *
 * @param[in] self      resource manager object handle
 * @return              @c true if resource has been freed, @c false otherwise
 **/
extern bool SvRBObjectFreeResource(SvRBObject self);

/**
 * Add resource manager object listener.
 *
 * @memberof SvRBObject
 *
 * @param[in] self                  resource manager object handle
 * @param[in] listener              handle to an object implementing @ref SvRBObjectListener
 * @param[out] errorOut             error info
 **/
extern void SvRBObjectAddListener(SvRBObject self,
                                  SvObject listener,
                                  SvErrorInfo *errorOut);

/**
 * Remove resource manager object listener.
 *
 * @memberof SvRBObject
 *
 * @param[in] self                  resource manager object handle
 * @param[in] listener              previously registered listener
 * @param[out] errorOut             error info
 **/
extern void SvRBObjectRemoveListener(SvRBObject self,
                                     SvObject listener,
                                     SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

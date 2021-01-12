/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RESOURCE_MANAGER_H_
#define QB_RESOURCE_MANAGER_H_

/**
 * @file QBResourceManager.h
 * @brief Resource manager service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <QBResourceManager/SvRBLocator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBResourceManager Resource manager service class
 * @ingroup QBResourceManagerLibrary
 * @{
 *
 * @link QBResourceManager @endlink is a service that loads and caches various
 * resources (bitmaps and fonts) used by UI components.
 * There is no public API for creating own instances of this service.
 * The default instance can be retrieved from the service registry,
 * it is registered with well-known name "ResourceManager".
 **/

/**
 * Resource manager class.
 * @class QBResourceManager QBResourceManager.h <QBResourceManager/QBResourceManager.h>
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 **/
typedef struct QBResourceManager_ *QBResourceManager;


/**
 * Get runtime type identification object representing QBResourceManager class.
 *
 * @relates QBResourceManager
 *
 * @return resource manager class
 **/
extern SvType
QBResourceManager_getType(void);

/**
 * Set file search path used for locating resources in the file system.
 *
 * This method operates on the instance of the @ref SvRBLocator used
 * internally by the resource manager.
 *
 * @memberof QBResourceManager
 *
 * @param[in] self      resource manager handle
 * @param[in] dirList   file search path as a list of directories
 *                      separated by colons (UNIX-style)
 * @param[in] argv0     executable name
 * @param[out] errorOut error info
 **/
extern void
QBResourceManagerSetSearchPaths(QBResourceManager self,
                                const char *dirList,
                                const char *argv0,
                                SvErrorInfo *errorOut);

/**
 * Get handle to the instance of the @ref SvRBLocator used by the resource manager.
 *
 * @memberof QBResourceManager
 *
 * @param[in] self      resource manager handle
 * @return              resource locator handle or @c NULL in case of error
 **/
extern SvRBLocator
QBResourceManagerGetResourceLocator(QBResourceManager self);

/**
 * Set the cache size limits.
 *
 * This method configures two cache size limits.
 *
 * When the amount of memory used for resources exceeds @a softLimit,
 * resource manager starts to remove objects having policy
 * set to #SvRBPolicy_auto.
 *
 * When the amount of memory used for resources exceeds @a hardLimit,
 * resource manager will not load any new resources having policy
 * set to #SvRBPolicy_auto.
 *
 * @memberof QBResourceManager
 *
 * @param[in] self      resource manager handle
 * @param[in] softLimit soft cache size limit (in bytes): when exceeded, unused resources will be released
 * @param[in] hardLimit hard cache size limit (in bytes): when exceeded, no more dynamic resources will be allowed to load
 * @param[out] errorOut error info
 **/
extern void
QBResourceManagerSetCacheSize(QBResourceManager self,
                              size_t softLimit,
                              size_t hardLimit,
                              SvErrorInfo *errorOut);

/**
 * Set SSL parameters used for downloading files over HTTPS.
 *
 * @memberof QBResourceManager
 *
 * @param[in] self      resource manager handle
 * @param[in] sslParams SSL parameters handle
 * @param[out] errorOut error info
 **/
extern void
QBResourceManagerSetSSLParams(QBResourceManager self,
                              SvSSLParams sslParams,
                              SvErrorInfo *errorOut);

/**
 * Set max size of the cache used by internal HTTP service.
 *
 * @memberof QBResourceManager
 *
 * @param[in] self      resource manager handle
 * @param[in] maxSize   maximum cache size in bytes
 * @param[out] errorOut error info
 **/
extern void
QBResourceManagerSetHTTPCacheSize(QBResourceManager self,
                                  off_t maxSize,
                                  SvErrorInfo *errorOut);

/**
 * Set upper limit on the size of downloaded files.
 *
 * @memberof QBResourceManager
 *
 * @param[in] self      resource manager handle
 * @param[in] maxSize   max download size in bytes
 * @param[out] errorOut error info
 **/
extern void
QBResourceManagerSetMaxDownloadSize(QBResourceManager self,
                                    off_t maxSize,
                                    SvErrorInfo *errorOut);

/**
 * Remove all cached resources.
 *
 * @memberof QBResourceManager
 *
 * @param[in] self      resource manager handle
 * @param[out] errorOut error info
 **/
extern void
QBResourceManagerRemoveAllResources(QBResourceManager self,
                                    SvErrorInfo *errorOut);

/**
 * Statistics collected by the resource manager service.
 **/
typedef struct {
    /// hardware memory (in bytes) used for bitmaps
    size_t bitmapMemoryUsed;
    /// hardware memory (in bytes) used for static bitmaps
    size_t staticBitmapMemoryUsed;
} QBResourceManagerStats;

/**
 * Get statistics collected by the resource manager service.
 *
 * @memberof QBResourceManager
 *
 * @param[in] self      resource manager handle
 * @param[out] stats    statistics filled by this method
 * @param[out] errorOut error info
 **/
extern void
QBResourceManagerGetStats(QBResourceManager self,
                          QBResourceManagerStats *const stats,
                          SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

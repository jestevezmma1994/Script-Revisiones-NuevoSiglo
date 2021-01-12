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

#ifndef SV_HTTP_FILE_CACHE_H_
#define SV_HTTP_FILE_CACHE_H_

/**
 * @file SvHTTPFileCache.h
 * @brief HTTP client file cache class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>


/**
 * @defgroup SvHTTPFileCache HTTP client file cache class
 * @ingroup SvHTTPClient
 * @{
 *
 * A utility class mapping URLs of downloaded files to local file paths.
 **/


/**
 * HTTP client file cache class.
 * @class SvHTTPFileCache
 * @extends SvObject
 **/
typedef struct SvHTTPFileCache_ *SvHTTPFileCache;

/**
 * HTTP client file cache entry class.
 * @ingroup SvHTTPFileCacheEntry
 * @class SvHTTPFileCacheEntry
 * @extends SvObject
 **/
typedef struct SvHTTPFileCacheEntry_ *SvHTTPFileCacheEntry;


/**
 * Get runtime type identification object representing
 * type of HTTP client file cache class.
 *
 * @return HTTP client file cache class
 **/
extern SvType
SvHTTPFileCache_getType(void);

/**
 * Initialize HTTP client file cache object.
 *
 * @memberof SvHTTPFileCache
 *
 * @param[in] self         HTTP client file cache handle
 * @param[in] dirPath      path to the directory where cached files will be stored
 * @param[in] sizeLimit    max total size of files stored in cache
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvHTTPFileCache
SvHTTPFileCacheInit(SvHTTPFileCache self,
                    SvString dirPath,
                    size_t sizeLimit,
                    SvErrorInfo *errorOut);

/**
 * Change size limit of the HTTP client file cache.
 *
 * @memberof SvHTTPFileCache
 *
 * @param[in] self         HTTP client file cache handle
 * @param[in] sizeLimit    max total size of files stored in cache
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPFileCacheSetSizeLimit(SvHTTPFileCache self,
                            size_t sizeLimit,
                            SvErrorInfo *errorOut);

/**
 * Generate name for new HTTP client file cache entry.
 *
 * @memberof SvHTTPFileCache
 *
 * @param[in] self         HTTP client file cache handle
 * @param[in] URL          source URL of the file
 * @param[out] errorOut    error info
 * @return                 path to the cached file
 **/
extern SvString
SvHTTPFileCacheGenerateName(SvHTTPFileCache self,
                            const char *URL,
                            SvErrorInfo *errorOut);

/**
 * Register new HTTP client file cache entry.
 *
 * This method creates new entry in the cache. It will only
 * exist as an in-memory object --- the corresponding file has
 * to be created explicitly.
 *
 * @memberof SvHTTPFileCache
 *
 * @param[in] self         HTTP client file cache handle
 * @param[in] URL          source URL of the file
 * @param[out] errorOut    error info
 * @return                 file cache entry, @c NULL in case of error
 **/
extern SvHTTPFileCacheEntry
SvHTTPFileCacheAddEntry(SvHTTPFileCache self,
                        const char *URL,
                        SvErrorInfo *errorOut);

/**
 * Find cached file in the HTTP client file cache.
 *
 * @memberof SvHTTPFileCache
 *
 * @param[in] self         HTTP client file cache handle
 * @param[in] URL          source URL of the file
 * @param[out] errorOut    error info
 * @return                 file cache entry, @c NULL if not found
 **/
extern SvHTTPFileCacheEntry
SvHTTPFileCacheFindEntry(SvHTTPFileCache self,
                         const char *URL,
                         SvErrorInfo *errorOut);

/**
 * Remove existing entry from the HTTP client file cache.
 *
 * @memberof SvHTTPFileCache
 *
 * @param[in] self         HTTP client file cache handle
 * @param[in] URL          source URL of the file
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPFileCacheRemoveEntry(SvHTTPFileCache self,
                           const char *URL,
                           SvErrorInfo *errorOut);

/**
 * Remove all registered entries from the cache directory.
 *
 * @memberof SvHTTPFileCache
 *
 * @param[in] self         HTTP client file cache handle
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPFileCacheFlush(SvHTTPFileCache self,
                     SvErrorInfo *errorOut);

/**
 * @}
 **/


/**
 * @defgroup SvHTTPFileCacheEntry HTTP client file cache entry class
 * @ingroup SvHTTPClient
 * @{
 *
 * A class representing single downloaded file cached in local file system.
 **/

/**
 * Check if an entry in HTTP client file cache is complete.
 *
 * @memberof SvHTTPFileCacheEntry
 *
 * @param[in] self         HTTP client file cache entry handle
 * @return                 @c true if entry describes completed HTTP transfer
 **/
extern bool
SvHTTPFileCacheEntryIsComplete(SvHTTPFileCacheEntry self);

/**
 * Get local path of a cached entry.
 *
 * @memberof SvHTTPFileCacheEntry
 *
 * @param[in] self         HTTP client file cache entry handle
 * @return                 path to a file in local filesystem
 **/
extern SvString
SvHTTPFileCacheEntryGetPath(SvHTTPFileCacheEntry self);

/**
 * @}
 **/


#endif

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DSMCC_CLIENT_INTERFACE_H
#define QB_DSMCC_CLIENT_INTERFACE_H

#include <SvFoundation/SvCoreTypes.h>
#include <stdint.h>
#define DSMCC_CACHE_DIR "/tmp/dsmcc"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * QBDSMCCClient handle
 **/
typedef struct QBDSMCCClient_s* QBDSMCCClient;

/**
 * ModuleCacheEntry handle
 **/
typedef struct ModuleCacheEntry_s* ModuleCacheEntry;

/**
 * QBDSMCCClient listener interface
 **/
typedef const struct QBDSMCCClientListener_ {
    /**
     * Method called when all DSM-CC data for the channel has been downloaded.
     *
     * @param[in] self_ listener handle
     **/
    void (*dataReady)(SvObject self_);
} *QBDSMCCClientListener;

SvInterface QBDSMCCClientListener_getInterface(void);

/**
 * File structure returned by QBDSMCCClientGetFile() call.
 **/
struct QBDSMCCClientFile_s {
    /**
     * SvObject structure.
     **/
    struct SvObject_ super_;
    /**
     * Pointer to file content. If file cannot be retrieved data is NULL.
     **/
    const unsigned char* data;
    /**
     * Length of file in bytes. If file cannot be retrieved lenght is 0.
     **/
    size_t length;
    /**
     * Module that this file belongs to. Module is retained for lifetime of QBDSMCCClientFile.
     **/
    ModuleCacheEntry parent;
};
typedef struct QBDSMCCClientFile_s* QBDSMCCClientFile;

SvType QBDSMCCClientFile_getType(void);

/**
 * Create QBDSMCCClientFile
 **/
QBDSMCCClientFile QBDSMCCClientFileCreate(void);

/**
 * Create QBDSMCCClient for particulat @a carouselId
 *
 * @param[in] carouselId identifier of DSM-CC carousel
 **/
QBDSMCCClient QBDSMCCClientCreate(uint32_t carouselId);

/**
 * Set cache size limit for DSM-CC client.
 *
 * @param[in] self DSM-CC client handle
 * @param[in] size maximum cache size in bytes
 **/
void QBDSMCCClientSetCacheSize(QBDSMCCClient self, size_t size);

/**
 * Add listener for DSM-CC client.
 *
 * @param[in] self DSM-CC client handle
 * @param[in] listener listener object to add
 **/
void QBDSMCCClientAddListener(QBDSMCCClient self, SvObject listener);

/**
 * Remove listener for DSM-CC client.
 *
 * @param[in] self DSM-CC client handle
 * @param[in] listener listener object to remove
 **/
void QBDSMCCClientRemoveListener(QBDSMCCClient self, SvObject listener);

/**
 * Pass section data to QBDSMCCClient
 *
 * @param[in] self DSM-CC client handle
 * @param[in] carouselId identifier of DSM-CC carousel
 * @param[in] section pointer to section data of @a sectionLength size
 * @param[in] sectionLength size of @a section data
 **/
void QBDSMCCClientSectionReceived(QBDSMCCClient self, uint32_t carouselId, const unsigned char* section, size_t sectionLength);

/**
 * Check if file exists in DSM-CC client's cache.
 *
 * @param[in] self DSM-CC client handle
 * @param[in] filePath requested file path
 * @return true if file exists
 */
bool QBDSMCCClientFileExists(QBDSMCCClient self, const char* filePath);

/**
 * Retrieve file from DSM-CC client's cache.
 *
 * @param[in] self DSM-CC client handle
 * @param[in] filePath requested file path
 * @return file or @a NULL in case of error
 **/
QBDSMCCClientFile QBDSMCCClientGetFile(QBDSMCCClient self, const char* filePath);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DSMCC_CLIENT_INTERFACE_H

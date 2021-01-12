/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RECORD_FS_META_AUTH_PLUGIN_H_
#define QB_RECORD_FS_META_AUTH_PLUGIN_H_

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvInterface.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBRecordFSMetaFileAuthPlugin.h QBRecordFSMetaFileAuthPlugin interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBRecordFS Record file system
 */

/**
 * @defgroup QBRecordFSMetaFileAuthPlugin QBRecordFSMetaFileAuthPlugin
 * Interface of metafile authentication plugin.
 *
 * @ingroup QBRecordFS
 * @{
 **/

/**
 * Callback is called when secured metadata has be created. Secured meta data can be stored directly on external storage.
 * @param[in] target callback target
 * @param[in] securedMetaData secured metadata which can be stored directly on external storage, @c NULL in case of error.
 */
typedef void (*QBRecordFSMetaFileAuthPluginGeneratedSecuredMetaFileCallback)(SvObject target, SvData securedMetaData);

/**
 * Callback is called when meta data from external storage is authenticated.
 * @param[in] target callback target
 * @param[in] authenticated @c true if metadata has been successfully authenticated, @c false if metadata has not been authenticated - it can not be used.
 */
typedef void (*QBRecordFSMetaFileAuthPluginAuthenticateMetaFileCallback)(SvObject target, const bool authenticated);

/**
 * QBRecordFSMetaFileAuthPlugin interface of meta file authentication plugin.
 */
typedef struct QBRecordFSMetaFileAuthPlugin_s {
    /**
     * Take new handle to auth token.
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSMetaFileAuthPlugin @endlink
     * @param[in] securedMetaFile data from secured metafile
     * @return    new handle to authentication token
     */
    SvData (*takeAuthToken)(SvObject self_, SvData securedMetaFile);
    /**
     * Take new handle to meta data
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSMetaFileAuthPlugin @endlink
     * @param[in] securedMetaFile data from secured metafile
     * @return    new handle to metadata
     */
    SvData (*takeMetaData)(SvObject self_, SvData securedMetaFile);
    /**
     * Generate secure metadata which can be stored directly on external storage
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSMetaFileAuthPlugin @endlink
     * @param[in] metadata to be secured
     * @param[in] target callback target
     * @param[in] callback callback which is called when operation is finished
     * @return    operation operation
     */
    SvObject (*generateSecuredMetaFile)(SvObject self_, SvData metaData, SvObject target, QBRecordFSMetaFileAuthPluginGeneratedSecuredMetaFileCallback callback);
    /**
     * Authenticate metadata read directly from external storage
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSMetaFileAuthPlugin @endlink
     * @param[in] metaData to be authenticated
     * @param[in] authToken authentication token
     * @param[in] target callback target
     * @param[in] callback callback which is called when operation is finished
     * @return    handle to operation
     */
    SvObject (*authenticateMetaFile)(SvObject self_, SvData securedMetaData, SvData authToken, SvObject target, QBRecordFSMetaFileAuthPluginAuthenticateMetaFileCallback callback);
} *QBRecordFSMetaFileAuthPlugin;

/**
 * Get runtime type identification object representing
 * QBRecordFSMetaFileAuthPlugin interface.
 *
 * @return QBRecordFSMetaFileAuthPlugin interface.
 **/
SvInterface QBRecordFSMetaFileAuthPlugin_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif //QB_RECORD_FS_META_AUTH_PLUGIN_H_

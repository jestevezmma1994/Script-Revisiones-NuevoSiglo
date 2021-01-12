/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RECORD_FS_DATA_ENCRYPTION_PLUGIN_H_
#define QB_RECORD_FS_DATA_ENCRYPTION_PLUGIN_H_

#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBRecordFSDataEncryptionPlugin.h QBRecordFSDataEncryptionPlugin interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBRecordFS Record file system
 */

/**
 * @defgroup QBRecordFSDataEncryptionPlugin QBRecordFSDataEncryptionPlugin
 * Interface of data encryption plugin.
 *
 * @ingroup QBRecordFS
 * @{
 **/

/**
 * QBRecordFSDataEncryptionPlugin callbacks
 */
typedef struct QBRecordFSDataEncryptionPluginCallbacks_ {
    /**
     * Set block independent metadata
     * Answer to QBRecordFSDataEncryptionPlugin::startEncryption. It also could be called later if metadata will change.
     *
     * @param[in] self_ callback target
     * @param[in] metadata block independent metadata or NULL in case of error
     */
    void (*setGlobalMetadata)(SvObject self_, SvObject metadata);
    /**
     * Set encrypted block and associated metadata
     * Answer to QBRecordFSDataEncryptionPlugin::encrypt
     *
     * @param[in] self_ callback target
     * @param[in] data encrypted block data or NULL in case of error
     * @param[in] blockMetadata block dependent metadata or NULL if not changed since last block
     */
    void (*setEncryptedData)(SvObject self_, SvData data, SvObject blockMetadata);
    /**
     * Set decrypted block
     * Answer to QBRecordFSDataEncryptionPlugin::decrypt
     *
     * @param[in] self_ callback target
     * @param[in] data decrypted block data or NULL in case of error
     */
    void (*setDecryptedData)(SvObject self_, SvData data);
} QBRecordFSDataEncryptionPluginCallbacks;

/**
 * QBRecordFSDataEncryptionPlugin interface of data encryption plugin.
 */
typedef struct QBRecordFSDataEncryptionPlugin_ {
    /**
     * Set plugin callbacks.
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSDataEncryptionPlugin @endlink
     * @param[in] callbacks plugin callbacks
     * @param[in] target callback target
     */
    void (*setCallbacks)(SvObject self_, const QBRecordFSDataEncryptionPluginCallbacks *callbacks, SvObject target);

    /**
     * Request start of encryption session.
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSDataEncryptionPlugin @endlink
     * @return @c 0 in case of success, @c -1 in case of error
     */
    int (*startEncryption)(SvObject self_);

    /**
     * Request stop of encryption session.
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSDataEncryptionPlugin @endlink
     */
    void (*stopEncryption)(SvObject self_);

    /**
     * Encrypt block of data. This should be called only between startEncryption/stopEncryption.
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSDataEncryptionPlugin @endlink
     * @param[in] data data to encrypt
     * @param[in] len length of data, must be 16 bytes aligned
     */
    void (*encrypt)(SvObject self_, const uint8_t *data, size_t len);

    /**
     * Request start of decryption session.
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSDataEncryptionPlugin @endlink
     * @param[in] globalMetadata block independent metadata
     * @return 0 in case of success, -1 in case of error
     */
    int (*startDecryption)(SvObject self_, SvObject globalMetadata);

    /**
     * Request stop of decryption session.
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSDataEncryptionPlugin @endlink
     */
    void (*stopDecryption)(SvObject self_);

    /**
     * Decrypt block of data. This should be called only between startDecryption/stopDecryptionn.
     *
     * @param[in] self_ handle to object which implements @link QBRecordFSDataEncryptionPlugin @endlink
     * @param[in] data data to decrypt
     * @param[in] len length of data, must be 16 bytes aligned
     * @param[in] blockMetadata metadata associated to encrypted data
     */
    void (*decrypt)(SvObject self_, const uint8_t *data, size_t len, SvObject blockMetadata);
} *QBRecordFSDataEncryptionPlugin;

/**
 * Get runtime type identification object representing
 * QBRecordFSDataEncryptionPlugin interface.
 *
 * @return QBRecordFSDataEncryptionPlugin interface.
 **/
SvInterface QBRecordFSDataEncryptionPlugin_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif //QB_RECORD_FS_DATA_ENCRYPTION_PLUGIN_H_

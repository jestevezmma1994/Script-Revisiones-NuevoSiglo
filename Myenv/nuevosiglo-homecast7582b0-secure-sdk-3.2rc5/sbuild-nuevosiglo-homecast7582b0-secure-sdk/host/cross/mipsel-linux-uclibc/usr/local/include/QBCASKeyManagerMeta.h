/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2013-2015 Cubiware Sp. z o.o. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau,
 ** application service provider, or similar business, or make any other use of
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef QBCAS_KEY_MANAGER_META_H_
#define QBCAS_KEY_MANAGER_META_H_

/**
 * @file QBCASKeyManagerMeta.h
 * @brief Manager of PVR reencryption keys.
 **/

#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBCASKeyManagerMeta  QBCASKeyManagerMeta service
 * @{
 **/

/**
 * QBCASKeyManagerMeta service
 *
 * PVR Reencryption needs several keys and this service allows to manage them.
 *
 * This class contain meta key, these keys are generated per PVR or are
 * provided by CAS vendors. Keys provided by CAS vendors have higher priority
 * than keys from PVR and they are never saved on PVR.
 * When PVR is inserted then keys should be loaded from meta partition.
 * Using this service is also possible to generate and write missing keys on PVR.
 *
 *
 * @class QBCASKeyManagerMeta
 */
typedef struct QBCASKeyManagerMeta_s* QBCASKeyManagerMeta;
/**
 * Create meta key manager instance.
 *
 * @return QBCASKeyManagerMeta handle of created instance
 */
QBCASKeyManagerMeta QBCASKeyManagerMetaCreate(void);
/**
 * Sets path to PVR meta root directory
 *
 * @param[in] self meta key manager
 * @param[in] metaRoot path to PVR meta root directory
 */
void QBCASKeyManagerMetaSetMetaRoot(QBCASKeyManagerMeta self, const char* metaRoot);
/**
 * Clear all loaded, generated and provided keys
 *
 * @param[in] self meta key manager
 */
void QBCASKeyManagerMetaClearKeys(QBCASKeyManagerMeta self);
/**
 * Load meta keys from PVR and write missing meta keys.
 *
 * @param[in] self meta key manager
 */
void QBCASKeyManagerMetaUpdatePVRKeys(QBCASKeyManagerMeta self);
/**
 * Set custom keys, these keys will never be saved on PVR and have higher priority when calling get keys.
 *
 * @param[in] self meta key manager
 * @param[in] keys hash table of (SvString keyname -> SvData keydata)
 */
void QBCASKeyManagerMetaUpdateCustomKeys(QBCASKeyManagerMeta self, SvHashTable keys);
/**
 * Generate keys for given names if keys of these names are missing
 *
 * @param[in] self meta key manager
 * @param[in] keyNames array of SvString which are names of keys that should be generated
 */
void QBCASKeyManagerMetaGenerateMissingKeys(QBCASKeyManagerMeta self, SvArray keyNames);
/**
 * Request of meta keys
 *
 * @param[in] self meta key manager
 * @param[in] keyNames names of keys which should be returned
 * @param[out] out hash table of (SvString keyname -> SvData keydata) which will be filled with keys for given keyNames
 * @return true if all requested keys was loaded successfully, false otherwise
 */
bool QBCASKeyManagerMetaGetKeys(QBCASKeyManagerMeta self, SvArray keyNames, SvHashTable outputKeys);

/**
 * Object which implements that interface will be notified about:
 * key cahanges
 */
typedef const struct QBCASKeyManagerMetaListener_ {
    /**
     * @brief It is called each time some key changes
     * @param[in] self_ listener instance
     * @param[in] keyName name of changed key
     */
    void (*keyChanged)(SvObject self_, SvString keyName);
} *QBCASKeyManagerMetaListener;

/**
 * Gets SvInterface instance of QBCASKeyManagerMetaListener
 */
SvInterface QBCASKeyManagerMetaListener_getInterface(void);

/**
 * Adds a new listener that will be notified about meta key changes
 *
 * @param[in] self meta key manager
 * @param[in] listener listener object
 */
void QBCASKeyManagerMetaAddListener(QBCASKeyManagerMeta self, SvObject listener);

/**
 * Removes Meta key manager listener
 *
 * @param[in] self meta key manager
 * @param[in] listener listener object
 */
void QBCASKeyManagerMetaRemoveListener(QBCASKeyManagerMeta self, SvObject listener);


/**
 * @}
 */

#ifdef  __cplusplus
}
#endif

#endif /* QBCAS_KEY_MANAGER_META_H_ */

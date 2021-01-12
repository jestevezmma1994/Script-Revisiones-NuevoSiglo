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

#ifndef QBCAS_KEY_MANAGER_CONTENT_H_
#define QBCAS_KEY_MANAGER_CONTENT_H_

/**
 * @file QBCASKeyManagerContent.h
 * @brief Manager of content reencryption keys.
 **/

#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvData.h>
#include <QBCASKeyManagerMeta.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBCASKeyManagerContent  QBCASKeyManagerContent service
 * @{
 **/

/**
 * QBCASKeyManagerContent manager
 *
 * PVR Reencryption needs several keys and this service allows to manage them.
 *
 * This class contains content keys, these keys are generated per content or
 * a part of content. Content keys service automatically update keys in scrambler.
 * Content keys service could run in two modes: input and force. In input mode
 * scrambling keys are changed when descrambling keys are changed. In force
 * mode it is possible to change keys in any time. If custom keys are not provided
 * then on each key change new random keys would be generated.
 *
 * @class QBCASKeyManagerContent
 */
typedef struct QBCASKeyManagerContent_s* QBCASKeyManagerContent;
/**
 * Key name of PVR reencryption meta key
 */
#define META_KEY_PVR_NAME "PVR_KEY"
/**
 * QBCASKeyManagerContent modes.
 **/
enum QBCASKeyManagerContentType_e {
    /** key change is synchronized with descrambling keys */
    QBCASKeyManagerContentType_input,
    /** key change is forced */
    QBCASKeyManagerContentType_force,
};
typedef enum QBCASKeyManagerContentType_e QBCASKeyManagerContentType;

/**
 * QBCASKeyManagerContent params used in auto change mode.
 **/
union QBCASKeyManagerContentAutoChangeParam_u {
    /** Params used in input mode */
    struct {
        /** QBCAS sessionId filter used in ecm_status callback */
        uint8_t sessionId;
        /** ECM pid value filter used in ecm_status callback. In case of -1 ecms from all pids are accepted*/
        int16_t ecmPid;
    } inputMode;
    /** Params used in force mode */
    struct {
        /** time interval between key changes */
        SvTime time;
    } forceMode;
};
typedef union QBCASKeyManagerContentAutoChangeParam_u QBCASKeyManagerContentAutoChangeParam;

/**
 * Pair of descrambler keys.
 **/
typedef struct QBCASKeyManagerContentKeyPair {
    /** odd key */
    SvData odd;
    /** even key */
    SvData even;
} QBCASKeyManagerContentKeyPair;

/**
 * QBCASKeyManagerContent scrambling modes.
 **/
typedef enum QBCASKeyManagerContentScramblingMode_e {
    /** Scrambler is producing odd packets */
    QBCASKeyManagerContentScramblingMode_odd,
    /** Scrambler is producing even packets */
    QBCASKeyManagerContentScramblingMode_even,
    /** We don't know what kind of packets are currently on scrambler output */
    QBCASKeyManagerContentScramblingMode_unknown,
} QBCASKeyManagerContentScramblingMode;

/**
 * Callback called when auto change mode changed key
 *
 * @param[in] target callback owner
 * @param[in] ecmPid ecm pid for which keys are intended, in case of @c -1 keys are intended for all ecm pids.
 * @param[in] keys generated keys, or @c NULL in case of failure
 */
typedef void (*QBCASKeyManagerContentKeyChangedCallback)(void* target, const int16_t ecmPid, const QBCASKeyManagerContentKeyPair* keys);
/**
 * Creates content key manager instance
 *
 * @param[in] descramblerContextId, descrambler session identificator where keys should be stored
 * @param[in] metaKeyManager, source of meta key, if NULL, @link QBCryptoKeysStore @endlink will be used
 * @param[in] type, type of key change mode
 * @param[in] scheduler used in callbacks, and timer in force mode, if NULL value from SvSchedulerGet will be used
 * @param[in] callback function called when new keys are generated
 * @param[in] target first parameter in callback
 * @return content key manager instance handle
 */
QBCASKeyManagerContent QBCASKeyManagerContentCreate(int descramblerContextId, QBCASKeyManagerMeta metaKeyManager, QBCASKeyManagerContentType type, SvScheduler scheduler, QBCASKeyManagerContentKeyChangedCallback callback, void* target);

/**
 * Request key change, update key in Scrambler and return it.
 * In case of success ContentKeyChangedCallback will be fired.
 *
 * @param[in] self content key manager instance
 * @param[in] inputKeys keys provided to descrambler, in input used to detect which key should be changed, ignored in force mode
 * @return true on success, false otherwise
 */
bool QBCASKeyManagerContentChangeKey(QBCASKeyManagerContent self, const QBCASKeyManagerContentKeyPair* inputKeys);
/**
 * This method allows to use provided key instead of automatically generated by content key manager
 *
 * @param[in] self content key manager instance
 * @param[in] keys keys that should be used in next key change
 */
bool QBCASKeyManagerContentSetCustomKey(QBCASKeyManagerContent self, const QBCASKeyManagerContentKeyPair* keys);
/**
 * Starts auto change mode, in this mode calling QBCASKeyManagerContentChangeKey is not needed and generated keys will be provided in callback.
 * In input mode key will be changed on each ecm_status callback with correct sessionId and QBCASEcmStatus_ok. So if on single session audio
 * and video are descrambled using different ECMs, then key will change on each audio key change and video key change.
 *
 * @param[in] self content key manager instance
 * @param[in] param type specific parameter
 */
void QBCASKeyManagerContentActivateAutoChange(QBCASKeyManagerContent self, QBCASKeyManagerContentAutoChangeParam param);

/**
 * Change key and request immediately scrambling with new key.
 * In case of success ContentKeyChangedCallback will be fired.
 * This method has sense only in force mode.
 *
 * @param[in] self content key manager instance
 * @param[out] outputKeys previous Control Words
 * @return true on success, false otherwise
 */
bool QBCASKeyManagerContentChangeKeyAndUseImmediately(QBCASKeyManagerContent self);

/**
 * Return current scrambler keys
 *
 * @param[in] self content key manager instance
 * @return pair of scrambler keys
 */
const QBCASKeyManagerContentKeyPair* QBCASKeyManagerContentGetCurrentKey(QBCASKeyManagerContent self);

/**
 * Return current scrambling mode.
 * This value is only valuable in force mode and it allows to find out what kind of packets are produced by scrambler (odd / even)
 *
 * @param[in] self content key manager instance
 * @return current scrambling mode
 */
QBCASKeyManagerContentScramblingMode QBCASKeyManagerContentGetCurrentScramblingMode(QBCASKeyManagerContent self);

/**
 * @}
 */

#ifdef  __cplusplus
}
#endif

#endif /* QBCAS_KEY_MANAGER_CONTENT_H_ */

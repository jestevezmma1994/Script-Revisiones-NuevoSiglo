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

#ifndef QBICSMARTCARDCACHEDPIN_H_
#define QBICSMARTCARDCACHEDPIN_H_

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBICSmartcardCachedPIN QBICSmartcardCachedPIN
 * @{
 **/

/**
 * @file QBICSmartcardCachedPIN.h Cached PIN class used for Conax Enhanved PVR
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * Cached PIN status
 **/
typedef enum QBICSmartcardCachedPINStatus_e {
    QBICSmartcardCachedPINStatus_unknown,     /**< unknown cached PIN status */
    QBICSmartcardCachedPINStatus_ok,          /**< cached PIN is ok */
    QBICSmartcardCachedPINStatus_wrong,       /**< cached PIN is not valid */
} QBICSmartcardCachedPINStatus;

/**
 * Cached PIN class
 **/
typedef struct QBICSmartcardCachedPIN_s {
    struct SvObject_ super_;              /**< base class */
    QBICSmartcardCachedPINStatus status;  /**< cached PIN status */
    char pin[4];                          /**< pin value, valid if status is ok */
} *QBICSmartcardCachedPIN;

/**
 * Creates @ref QBICSmartcardCachedPIN class instance
 *
 * @return instance of Cached PIN class, NULL if error
 */
extern QBICSmartcardCachedPIN QBICSmartcardCachedPINCreate(void);

/**
 * Loads PIN value from memory
 *
 * @param[in] self Cached PIN class instance
 * @return 0 on success, -1 on fail
 */
extern int QBICSmartcardCachedPINLoad(QBICSmartcardCachedPIN self);

/**
 * Sets PIN value, and saves it to memory
 *
 * @param[in] self Cached PIN class instance
 * @param[in] pin new PIN value
 * @param[in] status status of new pin
 * @return
 */
extern int QBICSmartcardCachedPINSetAndSave(QBICSmartcardCachedPIN self, const char pin[4], QBICSmartcardCachedPINStatus status);

/**
 * Get default PIN value.
 * @param[in] self Cached PIN class instance
 * @param[out] default PIN value
 */
extern void QBICSmartcardCachedPINGetDefault(QBICSmartcardCachedPIN self, char pin[4]);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBICSMARTCARDCACHEDPIN_H_ */

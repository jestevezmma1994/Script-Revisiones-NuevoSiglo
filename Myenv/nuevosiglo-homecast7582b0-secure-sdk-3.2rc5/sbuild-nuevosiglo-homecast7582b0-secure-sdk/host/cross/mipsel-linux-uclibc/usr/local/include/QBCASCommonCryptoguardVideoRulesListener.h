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

#ifndef QBCASCOMMONCRYPTOGUARDVIDEORULESLISTENER_H_
#define QBCASCOMMONCRYPTOGUARDVIDEORULESLISTENER_H_

#include <QBCASCommonCryptoguard.h>
#include <QBCASCommonCryptoguardVideoRules.h>

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCASCommonCryptoguardVideoRulesListener.h QBCASCommonCryptoguardVideoRulesListener class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @interface QBCASCommonCryptoguardVideoRulesListener
 * Object which implements that interface will be notified about:
 * Video Rules Changes
 */
typedef struct QBCASCommonCryptoguardVideoRulesListener_s *QBCASCommonCryptoguardVideoRulesListener;

/**
 * @struct QBCASCommonCryptoguardVideoRulesListener_s
 * @brief See @ref QBCASCommonCryptoguardVideoRulesListener
 */
struct QBCASCommonCryptoguardVideoRulesListener_s {
    /**
     * @brief when occurs new message should be displayed on the screen
     * @param[in] self_ instance of listener
     * @param[in] sessionId @c -1 if not session specific, in the other case session number
     * @param[in] message @ref QBCASCommonCryptoguardVideoRules
     */
    void (*videoRulesChanged)(SvObject self_, int8_t sessionId, QBCASCommonCryptoguardVideoRules videoRules);
};

/**
 * @brief Get instance of SvInterface related to @ref QBCASCommonCryptoguardVideoRulesListener.
 * @return Instance of SvInterface.
 */
SvInterface QBCASCommonCryptoguardVideoRulesListener_getInterface(void);

/**
 * Adds a new listener that will be notified when Video Rules was changed
 *
 * @param[in] self @ref QBCASCommonCryptoguard
 * @param[in] listener listener object
 */
void QBCASCommonCryptoguardVideoRulesListenerAdd(QBCASCommonCryptoguard self, SvObject listener);

/**
 * Removes VideoRules listener
 *
 * @param[in] self @ref QBCASCommonCryptoguard
 * @param[in] listener listener object
 */
void QBCASCommonCryptoguardVideoRulesListenerRemove(QBCASCommonCryptoguard self, SvObject listener);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBCASCOMMONCRYPTOGUARDVIDEORULESLISTENER_H_ */

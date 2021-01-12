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

#ifndef QB_UPGRADE_VERIFY_LISTENER_H_
#define QB_UPGRADE_VERIFY_LISTENER_H_

/**
 * @file QBUpgradeVerifyListener.h
 * @brief Upgrade verify listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBUpgradeVerifyListener Upgrade verify listener interface
 * @ingroup QBUpgrade
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <QBUpgrade/QBUpgradeVerifier.h>


/**
 * Upgrade verify listener interface.
 **/
typedef struct QBUpgradeVerifyListener_ {
    /**
     * Notify that verification progress have changed.
     *
     * @param[in] self_     listener handle
     * @param[in] verifier  upgrade verifier handle
     **/
    void (*progressChanged)(SvObject self_,
                            QBUpgradeVerifier verifier);
} *QBUpgradeVerifyListener;


/**
 * Get runtime type identification object
 * representing QBUpgradeVerifyListener interface.
 *
 * @return QBUpgradeVerifyListener interface object
 **/
extern SvInterface
QBUpgradeVerifyListener_getInterface(void);

/**
 * @}
 **/


#endif

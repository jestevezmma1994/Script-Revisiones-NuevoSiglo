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

#ifndef QBPARENTALCONTROLLISTENER_H
#define QBPARENTALCONTROLLISTENER_H

/**
 * @file QBParentalControlListener.h
 * @brief Parental control service listener interface.
 **/

#include <Services/QBParentalControl/QBParentalControl.h>
#include <QBPCRatings/QBPCList.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>

typedef struct QBParentalControlListener_s {

    void (*ratingChanged)(SvObject self_, QBPCList ratingList);

    void (*stateChanged)(SvObject self_, QBParentalControlState state);

    /**
     * Notify that new locked time task has been added.
     *
     * @param[in] self_     listener handle
     * @param[in] task      added task
     **/
    void (*lockedTimeTaskAdded)(SvObject self_, QBParentalControlLockedTimeTask task);

    /**
     * Notify that locked time task has been removed.
     *
     * @param[in] self_     listener handle
     * @param[in] task      removed task
     **/
    void (*lockedTimeTaskRemoved)(SvObject self_, QBParentalControlLockedTimeTask task);
} *QBParentalControlListener;

SvInterface QBParentalControlListener_getInterface(void);

#endif // QBPARENTALCONTROLLISTENER_H

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBVIEWRIGHT_H_
#define QBVIEWRIGHT_H_

#include <QBViewRightOSM.h>
#include <QBViewRightTriggers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>

#include <fibers/c/fibers.h>
#include <dataformat/sv_data_format.h>
#include <SvPlayerKit/SvECM.h>

/**
 * @class QBViewRight
 * @brief QBViewRight is used for communicating between the application and ViewRight library (bcrypt2)
 * QBViewRight is a singleton.
 */
typedef struct QBViewRight_t *QBViewRight;
/**
 * QBViewRight is a singleton. This global method returns instance of QBViewRight.
 * @return QBViewRight instance
 */
QBViewRight QBViewRightGetInstance(void);

SvType QBViewRight_getType(void);
/**
 *
 * @param self
 * @param listener
 */
void QBViewRightTriggerListenerAdd(QBViewRight self, SvObject listener);
/**
 *
 * @param self
 * @param listener
 */
void QBViewRightTriggerListenerRemove(QBViewRight self, SvObject listener);
/**
 *
 * @param self
 * @param listener
 */
void QBViewRightOsmListenerAdd(QBViewRight self, SvObject listener);
/**
 *
 * @param self
 * @param listener
 */
void QBViewRightOsmListenerRemove(QBViewRight self, SvObject listener);

void QBViewRightPinStatusListenerAdd(QBViewRight self, SvObject listener);

void QBViewRightPinStatusListenerRemove(QBViewRight self, SvObject listener);

void QBViewRightCopyControlListenerAdd(QBViewRight self, SvObject listener);

void QBViewRightCopyControlListenerRemove(QBViewRight self, SvObject listener);

/**
 *
 * @param self
 * @param canShow
 */
void QBViewRightCanShowOSM(QBViewRight self, bool canShow);

typedef enum QBViewRightPinStatus_e {
    QBViewRightPinStatus_OK,
    QBViewRightPinStatus_Invalid,
    QBViewRightPinStatus_ErrorOccured,
    QBViewRightPinStatus_NotEnoughPurse,
    QBViewRightPinStatus_Blocked
} QBViewRightPinStatus;

typedef enum QBViewRightPinSelect_e {
    QBViewRightPinSelect_NonSmartcard,
    QBViewRightPinSelect_Smartcard
} QBViewRightPinSelect;

typedef struct QBViewRightPinStatusListener_i {
    void (*pinStatus)(void *self_, QBViewRightPinStatus pinStatus);
} *QBViewRightPinStatusListener;

typedef struct QBViewRightState_s  *QBViewRightState;

typedef struct QBViewRightStateListner_i {
    void (*stateChanged)(void *self_, QBViewRightState);
} *QBViewRightStateListner;

SvInterface QBViewRightPinStatusListener_getInterface(void);

/**
 * @brief Returns information if non smartcard is enabled
 * @param self instance of QBViewRight
 * @return @c true if nonsmartcard supported
 */
bool QBViewRightIsNonSmartcardEnabled(QBViewRight self);

/**
 * @brief Returns information if smartcard is enabled
 * @param self instance of QBViewRight
 * @return @c true if smartcard is present and enabled
 */
bool QBViewRightIsSmartcardEnabled(QBViewRight self);

/**
 * @brief Enter pin if it was requested by library, to get access to content
 * @param[in] self instance of QBViewRight
 * @param[in] pinListener pin listener which implements QBViewRightStateListner interface
 * @param[in] pinType pin type
 * @param[in] pin string with pin (i.e '5372')
 */
void QBViewRightEnterPin(QBViewRight self, SvObject pinListener, QBViewRightPinType pinType, SvString pin);

/**
 * @brief Check internal pin of nonsmarcard or smartcard
 * @param[in] self instance of QBViewRight
 * @param[in] pinListener pin listener which implements QBViewRightStateListner interface
 * @param[in] pinSelect pin select - smartcard or nonsmartcard
 * @param[in] pin string with pin (i.e '5372')
 */
void QBViewRightCheckPin(QBViewRight self, SvObject pinListener, QBViewRightPinSelect pinSelect, SvString pin);

/**
 * This method is very dangerous. It shall never be called by main application.
 * Use it only if you know exactly what you are doing.
 * It cleans individualization data banks at STB.
 * It doesn't remove individulized marker, since it is written in OTP memory.
 * Method should be used before indvidualization process to remove some data from previous failed individualization.
 */
void QBViewRightCleanIndividualizationData(void);

#endif /* QBVIEWRIGHT_H_ */

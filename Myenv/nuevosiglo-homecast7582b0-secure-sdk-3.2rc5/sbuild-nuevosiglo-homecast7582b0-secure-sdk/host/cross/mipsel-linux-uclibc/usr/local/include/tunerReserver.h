/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBTUNER_RESERVER_H_
#define QBTUNER_RESERVER_H_

/**
 * @file tunerReserver.h
 * @brief Tuner reservation manager.
 *
 * Provides access to all DVB tuners availabe.
 * No module should use tuner until obtaining from QBTunerReserver.
 */

#include <stdbool.h>
#include <SvFoundation/SvString.h>
#include <QBTunerTypes.h>

/**
 * @brief Priorities for different services used by \ref QBTunerResv.
 */
enum {
    QBTUNERRESERVER_PRIORITY_FORCE_UPGRADE = 120,
    QBTUNERRESERVER_PRIORITY_SATELLITE_SETUP = 110,
    QBTUNERRESERVER_PRIORITY_PVR = 100,
    QBTUNERRESERVER_PRIORITY_SATELLITE_CHECK = 98,
    QBTUNERRESERVER_PRIORITY_CHANNEL_SCAN = 95,
    QBTUNERRESERVER_PRIORITY_TUNER_STATUS_CHECK = 94,
    QBTUNERRESERVER_PRIORITY_VOD = 92,
    /** It is used to check if it will be possible to obtain tuner in case of removal all BACKGROUND_PLAYER reservations */
    QBTUNERRESERVER_PRIORITY_BACKGROUND_PLAYER_CHECK = 91,
    QBTUNERRESERVER_PRIORITY_BACKGROUND_PLAYER = 90,
    QBTUNERRESERVER_PRIORITY_STANDBY = 85,
    QBTUNERRESERVER_PRIORITY_PUSHVOD = 80,
    QBTUNERRESERVER_PRIORITY_UPGRADE = 40,
};

#define QBTUNERRESERVER_PRIORITY_EPG_MONITOR_BASE  29
#define QBTUNERRESERVER_PRIORITY_EPG_MONITOR_JUMP   2

/**
 * @brief Typedef for reservation's parameters \ref QBTunerResvParams_s.
 */
typedef struct QBTunerResvParams_s QBTunerResvParams;
/**
 * @brief Typedef for reservation's callbacks \ref QBTunerResvCallbacks_s.
 */
typedef struct QBTunerResvCallbacks_s QBTunerResvCallbacks;
/**
 * @brief Typedef for tuner reservation struct \ref QBTunerResv_s.
 */
typedef struct QBTunerResv_s QBTunerResv;

/**
 * @class QBTunerReserver tunerReserver.h <tunerReserver.h>
 * @brief Base tuner reserver class.
 *
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 *
 * It holds all data required to manage tuner reservation service.
 * The service can be retrieved from the service registry, in which it is
 * present under the name "QBTunerReserver".
 **/
typedef struct QBTunerReserver_s QBTunerReserver;

/**
 * @brief Tuner revoked callback.
 *
 * This callback is called any time reservation loses its tuner.
 * After that, tuninig parameters are invalid for revoked reservation and should not be changed.
 *
 * @param[in] self_ callback's data
 * @param[in] reserver \ref QBTunerReserver object that created action
 * @param[in] resv reservation which tuner was revoked
 */
typedef void (*QBTunerResvTunerRevoked)(void *self_, QBTunerReserver* reserver, QBTunerResv* resv);
/**
 * @brief Tuner obtained callback.
 *
 * This callback is called any time reservation obtains tuner and can set its
 * tuning parameters.
 *
 * @param[in] self_ callback's data
 * @param[in] reserver \ref QBTunerReserver object that created action
 * @param[in] resv reservation which obtained tuner
 * @param[in] tunerNum number of obtained tuner
 */
typedef void (*QBTunerResvTunerObtained)(void *self_, QBTunerReserver* reserver, QBTunerResv* resv, int tunerNum);

/**
 * @brief The QBTunerResvCallbacks_s struct holds callbacks information.
 *
 * \see QBTunerResvTunerRevoked
 * \see QBTunerResvTunerObtained
 */
struct QBTunerResvCallbacks_s {
    QBTunerResvTunerRevoked tunerRevoked;
    QBTunerResvTunerObtained tunerObtained;
};

/**
 * @brief The QBTunerResvParams_s struct keeps parameters required for reservation.
 */
struct QBTunerResvParams_s {
    int priority;                       /**< Priority of reservation */
    struct QBTunerParams tunerParams;   /**< Tuning parameters */
    bool tunerOwnership;                /**< Indicating that reservation takes ownership on tuner.
                                          \warning Any other service (even when it has the same tuning params) can't get access. Only service with higher priority can revoke tuner.
                                          \warning If master tuner set reservation takes ownership on all tuner */
    bool allTuners;                     /**< Reservation require all available tuners */
    bool immediate;                     /**< not used */
};

/**
 * @brief The QBTunerResv_s struct holds reservation data.
 */
struct QBTunerResv_s {
    struct SvObject_ super_;    /**< Base class object */
    void *callbackData;                 /**< Callbacks' data pointer */
    QBTunerResvCallbacks callbacks;     /**< Action callbacks \see QBTunerResvCallbacks_s */
    SvString name;                      /**< Reservation's debug name */

    QBTunerResvParams params;           /**< Reservation's parameters \see QBTunerResvParams_s */
};

/**
 * @brief QBTunerResvCreate creates reservation object \ref QBTunerResv.
 *
 * Create reservation object with given name and callbacks that can be used for
 * \ref QBTunerReserver API.
 * Reservaion parameters should be provided separately through structure member \ref QBTunerResv_s
 *
 * @param[in] name debug name of reservation
 * @param[in] callbackData callbacks' data
 * @param[in] callbacks action callbacks \see QBTunerResvCallbacks
 * @return Tuner reservation object or NULL in case of error
 */
QBTunerResv* QBTunerResvCreate(SvString name, void *callbackData, const QBTunerResvCallbacks *callbacks);

/**
 * Get runtime type identification object representing QBTunerReserver class.
 *
 * @relates QBTunerReserver
 *
 * @return QBTunerReserver class
 **/
SvType QBTunerReserver_getType(void);

/**
 * @brief QBTunerReserverCreate creates new, empty \ref QBTunerReserver object.
 *
 * This object should be passed as sefl parameter for any other public API.
 *
 * @return \ref QBTunerReserver object or NULL in case of error
 */
QBTunerReserver* QBTunerReserverCreate(void);

/**
 * @brief QBTunerReserverAddTuner adds new tuner for \ref QBTunerReserver.
 *
 * @param[in] self \ref QBTunerReserver object
 * @param[in] tunerNum number of new tuner
 */
void QBTunerReserverAddTuner(QBTunerReserver* self, int tunerNum);

/**
 * @brief QBTunerReserverTryObtain obtains tuner if possible.
 *
 * This function obtains tuner if it is possible and takes no action otherwise.
 *
 * @param[in] self \ref QBTunerReserver object
 * @param[in] resv \ref QBTunerResv object with reservation data
 * @return tuner number on success or negative value if it's not possible to obtain
 * tuner at the particular moment
 */
int QBTunerReserverTryObtain(QBTunerReserver* self, QBTunerResv* resv);

/**
 * @brief QBTunerReserverObtain obtains tuner or queues reservation.
 *
 * This function obtains tuner if it is possible or adds reservation to the awaiting
 * queue otherwise. When possibility for obtaining tuner occur \ref QBTunerResvTunerObtained
 * will be called for this reservation.
 *
 * @param[in] self \ref QBTunerReserver object
 * @param[in] resv \ref QBTunerResv object with reservation data
 * @return tuner number if tuner obtained or negative value when resv queued
 */
int QBTunerReserverObtain(QBTunerReserver* self, QBTunerResv* resv);

/**
 * @brief QBTunerReserverCanObtain checks if it is possible to obtain tuner at the moment.
 *
 * No change for \ref QBTunerReserver is made.
 *
 * @param[in] self \ref QBTunerReserver object
 * @param[in] resv \ref QBTunerResv object with reservation data
 * @param[in] params replacing params used for check. If NULL provided, resv->params will be used
 * @return true or false
 */
bool QBTunerReserverCanObtain(QBTunerReserver* self, QBTunerResv* resv, QBTunerResvParams* params);

/**
 * @brief QBTunerReserverRelease releases tuner reservation.
 *
 * Remove tuner reservation and all data regarding it.
 * If resv possess tuner it will be no longer available for it.
 *
 * @param[in] self \ref QBTunerReserver object
 * @param[in] resv \ref QBTunerResv object with reservation data
 */
void QBTunerReserverRelease(QBTunerReserver* self, QBTunerResv* resv);

/**
 * @brief QBTunerReserverStartTransaction starts internal transation.
 *
 * Transactions are used to perform several operation without internal tuners reassigning.
 * If any tuner will be released during transaction it will not be assign
 * to awaiting reservations until transaction ends.
 *
 * @param[in] self \ref QBTunerReserver object
 */
void QBTunerReserverStartTransaction(QBTunerReserver* self);

/**
 * @brief QBTunerReserverEndTransaction ends transaction and assigns free tuners.
 *
 * @param[in] self \ref QBTunerReserver object
 */
void QBTunerReserverEndTransaction(QBTunerReserver* self);

/**
 * @brief QBTunerReserverSetMasterTuner sets index of master tuner.
 *
 * This function is used for boxes with more than one tuner,
 * while all tuners are connected. This can be i.e. satelite box with loop back tuner.
 * When masterTuner is set, all other tuners can be assign to reservations that
 * are compatible with reservation on master tuner. Compatibility, in this case,
 * means the same satelite, same polarization and the frequency in the same band.
 *
 * @param[in] self \ref QBTunerReserver object
 * @param[in] masterTunerNum index of tuner to be set as master
 */
void QBTunerReserverSetMasterTuner(QBTunerReserver* self, int masterTunerNum);

#endif

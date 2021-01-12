/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

/* App/Libraries/QBDSMCC/Headers/QBDSMCC/QBDSMCCAdsRotationService.h */

#ifndef QB_DSMCC_ADS_ROTATION_SERVICE_H
#define QB_DSMCC_ADS_ROTATION_SERVICE_H

/**
 * @file QBDSMCCAdsRotationService.h
 * @brief DSM-CC Ads Rotation Service API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBDSMCCAdsRotationService DSM-CC Ads Rotation Service
 * @ingroup QBDSMCC
 * @{
 **/

/**
 * QBDSMCCAdsRotationService class.
 *
 * This class is a service that manages displaying times for given
 * advertisements (see @ref QBDSMCCAdsRotationServiceSetContent). The
 * advertisements are displayed in a loop.
 *
 * When time comes to display a set of ads, it's sent to listeners by the
 * @ref QBDSMCCAdsRotationServiceListener::setAds callback. If ads should be
 * removed, listeners are notified with @c NULL.
 **/
typedef struct QBDSMCCAdsRotationService_s *QBDSMCCAdsRotationService;

/**
 * Create QBDSMCCAdsRotationService
 * @return QBDSMCCAdsRotationService handle or NULL on error
 **/
QBDSMCCAdsRotationService QBDSMCCAdsRotationServiceCreate(void);

/**
 * Set new ads.
 * @param[in] self      QBDSMCCAdsRotationService handle
 * @param[in] content   (details below)
 *
 * Parameter content is a hash table with the following structure or @c NULL.
 *
 *     {
 *         "uploadTime": int - number of seconds in UTC timestamp,
 *         "period": int - adverts loop time in minutes,
 *         "desc": [
 *             {
 *                 "startTime": int - minute in loop in which this set should be displayed,
 *                 "endTime": int - minute in loop in which this set should be hidden,
 *                 "images": SvHashTable with images to display
 *             },
 *             {
 *                 ...
 *             }
 *         ]
 *     }
 *
 * The following sould apply: 0 <= desc[i].startTime < desc[i].endTime <=
 * desc[i+1].startTime.
 *
 * If there's set i where desc[i-1].startTime >= desc[i].startTime or
 * desc[i].startTime >= period then it's removed.
 *
 * If there's set i where desc[i].endTime > desc[i+1].startTime then it's
 * adjusted desc[i].endTime = desc[i+1].startTime.
 *
 * If there's set i where desc[i].endTime > period then it's adjusted
 * desc[i].endTime = period.
 *
 * If @c NULL was passed then service behaves as if it was deactivated.
 **/
void QBDSMCCAdsRotationServiceSetContent(QBDSMCCAdsRotationService self, SvHashTable content);

/**
 * Clear content.
 *
 * Has the same effect as calling SetContent(self, NULL);
 *
 * @param[in] self      QBDSMCCAdsRotationService handle
 */
void QBDSMCCAdsRotationServiceClearContent(QBDSMCCAdsRotationService self);

/**
 * Start QBDSMCCAdsRotationService
 * @param[in] self      QBDSMCCAdsRotationService handle
 * @param scheduler     scheduler under which the service fiber should run
 **/
void QBDSMCCAdsRotationServiceStart(QBDSMCCAdsRotationService self, SvScheduler scheduler);

/**
 * Stop QBDSMCCAdsRotationService
 * @param[in] self      QBDSMCCAdsRotationService handle
 **/
void QBDSMCCAdsRotationServiceStop(QBDSMCCAdsRotationService self);

/**
 * Add listener to QBDSMCCAdsRotationService
 * @param[in] self      QBDSMCCAdsRotationService handle
 * @param[in] listener  listener that implements
 *                      QBDSMCCAdsRotationServiceListener interface
 **/
void QBDSMCCAdsRotationServiceAddListener(QBDSMCCAdsRotationService self, SvObject listener);

/**
 * Remove listener from QBDSMCCAdsRotationService
 * @param[in] self      QBDSMCCAdsRotationService handle
 * @param[in] listener  listener
 **/
void QBDSMCCAdsRotationServiceRemoveListener(QBDSMCCAdsRotationService self, SvObject listener);

#if SV_LOG_LEVEL > 0
/**
 * Get internal timer
 *
 * THIS FUNCTION IS ONLY FOR UNIT TESTS!!!
 *
 * @param[in] self      QBDSMCCAdsRotationService handle
 * @return              timer
 */
SvFiberTimer QBDSMCCAdsRotationServiceGetTimer(QBDSMCCAdsRotationService self);
#endif

/**
 * QBDSMCCAdsRotationServiceListener interface
 **/
typedef const struct QBDSMCCAdsRotationServiceListener_ {
    /**
     * @param[in] self_     listener handle
     * @param[in] images    HashTable with images to display or @c NULL if ads
     *                      should be removed.
     **/
    void (*setAds)(SvObject self_, SvHashTable images);
} *QBDSMCCAdsRotationServiceListener;

/**
 * Get runtime type identification object representing
 * AdsRotationService listener interface.
 *
 * @return  AdsRotationService listener interface identification object
 **/
SvInterface QBDSMCCAdsRotationServiceListener_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_DSMCC_ADS_ROTATION_SERVICE_H */

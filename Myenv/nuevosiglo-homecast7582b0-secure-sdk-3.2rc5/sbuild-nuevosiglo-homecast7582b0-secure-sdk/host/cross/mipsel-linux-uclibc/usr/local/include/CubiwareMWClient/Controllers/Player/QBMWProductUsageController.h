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

#ifndef QB_MW_USE_PRODUCT_H_
#define QB_MW_USE_PRODUCT_H_

/**
 * @file    QBMWProductUsageController.h
 * @brief   Product usage controller API
 **/

#include <Services/core/QBMiddlewareManager.h>

#include <stdlib.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>

typedef enum {
    QBMWProductUsageControllerRequestState_created = 0,
    QBMWProductUsageControllerRequestState_using,
    QBMWProductUsageControllerRequestState_useEntitled,
    QBMWProductUsageControllerRequestState_useDenied,
    QBMWProductUsageControllerRequestState_ending,
    QBMWProductUsageControllerRequestState_useEnded,
    QBMWProductUsageControllerRequestState_canceled,
    QBMWProductUsageControllerRequestState_error
} QBMWProductUsageControllerRequestState;

/**
 * QBMWProductUsageListener interface.
 **/
struct QBMWProductUsageListener_t {
    /**
     * Notify that state of request has changed.
     *
     * @param[in] self_     handle to an object implementing @ref QBMWProductUsageListener
     * @param[in] requestID request identifier
     * @param[in] state     state of request
     **/
    void (*stateChanged)(SvObject self_, SvString requestID, QBMWProductUsageControllerRequestState state);

    /**
     * Notify that got answer for request.
     *
     * @param[in] self_     handle to an object implementing @ref QBMWProductUsageListener
     * @param[in] answer    received answer for request
     **/
    void (*gotAnswer)(SvObject self_, SvObject answer);

    /**
     * Notify that request has been finished.
     *
     * @param[in] self_     handle to an object implementing @ref QBMWProductUsageListener
     **/
    void (*requestFinished)(SvObject self_);
};
typedef struct QBMWProductUsageListener_t *QBMWProductUsageListener;

/**
 * Get runtime type identification object representing product usage listener interface.
 **/
extern SvInterface
QBMWProductUsageListener_getInterface(void);

/**
 * @defgroup QBMWProductUsageController Product usage controller class
 * @ingroup  CubiTV_services
 * @{
 **/
typedef struct QBMWProductUsageController_t *QBMWProductUsageController;

/**
 * Create product usage controller service.
 *
 * @param[in] middlewareManager     QBMiddlewareManager handle (usually found in AppGlobals)
 * @return                          new product usage controller service instance or @c NULL
 *                                  in case of error
 **/
QBMWProductUsageController
QBMWProductUsageControllerCreate(QBMiddlewareManager middlewareManager);

/**
 * Send stb.UseProduct request for given product.
 *
 * @param[in] self      product usage controller handle
 * @param[in] listener  handle to an object implementing @ref QBMWProductUsageListener
 * @param[in] product   product (VOD asset, TV channel) or product identifier for which
 *                      request will be send
 * @return              request identifier or @c empty string in case of invalid product
 **/
SvString
QBMWProductUsageControllerUseProduct(QBMWProductUsageController self, SvObject listener, SvObject product);

/**
 * Send stb.UseProduct request with specific mode for given product.
 *
 * @param[in] self      product usage controller handle
 * @param[in] listener  handle to an object implementing @ref QBMWProductUsageListener
 * @param[in] product   product (VOD asset, TV channel) or product identifier for which
 *                      request will be send
 * @param[in] mode      type of mode (startOver or startCatchup) for which request will be send
 * @return              request identifier or @c empty string in case of invalid product
 **/
SvString
QBMWProductUsageControllerUseProductWithMode(QBMWProductUsageController self, SvObject listener, SvObject product, const char *mode);

/**
 * Send stb.UseProduct request with license flag for given product.
 *
 * @param[in] self          product usage controller handle
 * @param[in] listener      handle to an object implementing @ref QBMWProductUsageListener
 * @param[in] product       product (VOD asset, TV channel) or product identifier for which
 *                          request will be send
 * @param[in] haveLicense   indicates that device has valid license for product.
 *                          If set to true, Portal server won’t contact Conax Contego system for AuthenticationData - token
 * @return                  request identifier or @c empty string in case of invalid product
 **/
SvString
QBMWProductUsageControllerUseProductWithLicenseFlag(QBMWProductUsageController self, SvObject listener, SvObject product, bool haveLicense);


/**
 * Send stb.UseProduct request with specific mode and license flag for given product.
 *
 * @param[in] self          product usage controller handle
 * @param[in] listener      handle to an object implementing @ref QBMWProductUsageListener
 * @param[in] product       product (VOD asset, TV channel) or product identifier for which
 *                          request will be send
 * @param[in] mode          type of mode (startOver or startCatchup) for which request will be send
 * @param[in] haveLicense   indicates that device has valid license for product.
 *                          If set to true, Portal server won’t contact Conax Contego system for AuthenticationData - token
 * @return                  request identifier or @c empty string in case of invalid product
 **/
SvString
QBMWProductUsageControllerUseProductWithModeAndLicenseFlag(QBMWProductUsageController self, SvObject listener, SvObject product, const char *mode, bool haveLicense);

/**
 * Send stb.EndProductUsage request for given request identifier.
 *
 * @param[in] self      product usage controller handle
 * @param[in] id        request identifier
 **/
void
QBMWProductUsageControllerEndProductUsage(QBMWProductUsageController self, SvString id);

/**
 * Send stb.UseProduct request for receive EMM for given product.
 *
 * @param[in] self      product usage controller handle
 * @param[in] listener  handle to an object implementing @ref QBMWProductUsageListener
 * @param[in] product   product (VOD asset, TV channel) or product identifier for which
 *                      request will be send
 **/
void
QBMWProductUsageControllerGetEMM(QBMWProductUsageController self, SvObject listener, SvObject product);

/**
 * Get cache answer for given product.
 *
 * @param[in] self      product usage controller handle
 * @param[in] product   product (VOD asset, TV channel) or product identifier
 * return               answer for given product or @c NULL in not found
 **/
SvObject
QBMWProductUsageControllerGetCachedAnswer(QBMWProductUsageController self, SvObject product);

/**
 * Add answer for given product to cache.
 *
 * @param[in] self      product usage controller handle
 * @param[in] product   product (VOD asset, TV channel) or product identifier
 * @param[in] answer    answer for request for given product
 **/
void
QBMWProductUsageControllerAddAnswerToCache(QBMWProductUsageController self, SvObject product, SvObject answer);

/**
 * @}
 **/

#endif


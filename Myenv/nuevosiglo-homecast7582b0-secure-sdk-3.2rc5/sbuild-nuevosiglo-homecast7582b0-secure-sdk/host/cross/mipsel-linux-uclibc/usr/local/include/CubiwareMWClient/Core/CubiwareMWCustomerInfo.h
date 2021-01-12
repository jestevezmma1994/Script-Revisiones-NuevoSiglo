/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUBIWAREMWCUSTOMERINFO_H_
#define CUBIWAREMWCUSTOMERINFO_H_

/**
 * @file CubiwareMWCustomerInfo.h
 * @brief Cubiware MW customer info class
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <Services/core/QBMiddlewareManager.h>

#include <stdbool.h>

/**
 * @defgroup CubiwareMWCustomerInfo Customer info class
 * @ingroup CubiwareMWClient
 **/

/**
 * Customer data status.
 **/
typedef enum {
    CubiwareMWCustomerInfoStatus_unknown = -1,
    CubiwareMWCustomerInfoStatus_ok = 0,
    CubiwareMWCustomerInfoStatus_error,
} CubiwareMWCustomerInfoStatus;

typedef const struct CubiwareMWCustomerInfoListener_ {
    void (*customerInfoChanged)(SvObject self_, SvString customerId, SvHashTable customerInfo);
    void (*statusChanged)(SvObject self_);
} *CubiwareMWCustomerInfoListener;

/**
 * Get runtime type identification object representing
 * CubiwareMWCustomerInfoListener interface.
 **/
extern SvInterface
CubiwareMWCustomerInfoListener_getInterface(void);

/**
 * CubiwareMWCustomerInfo type
 */
typedef struct CubiwareMWCustomerInfo_ *CubiwareMWCustomerInfo;

/**
 * Get runtime type identification object representing CubiwareMWCustomerInfo class.
 * @param[in] middlewareManager     QBMiddlewareManager handle
 * @param[in] refreshTimeSec        auto refresh delay time in seconds
 * @return                          CubiwareMWCustomerInfo handle
 **/
extern CubiwareMWCustomerInfo
CubiwareMWCustomerInfoCreate(QBMiddlewareManager middlewareManager, unsigned int refreshTimeSec);

/**
 * Start CustomerInfo service.
 * @param[in] self      CubiwareMWCustomerInfo handle
 */
extern void
CubiwareMWCustomerInfoStart(CubiwareMWCustomerInfo self);

/**
 * Stop CustomerInfo service.
 * @param[in] self      CubiwareMWCustomerInfo handle
 */
extern void
CubiwareMWCustomerInfoStop(CubiwareMWCustomerInfo self);

/**
 * Add CustomerInfo service listener.
 * @param[in] self      CubiwareMWCustomerInfo handle
 * @param[in] listener  listener of the CustomerInfo service
 */
extern void
CubiwareMWCustomerInfoAddListener(CubiwareMWCustomerInfo self, SvObject listener);

/**
 * Remove CustomerInfo service listener.
 * @param[in] self      CubiwareMWCustomerInfo handle
 * @param[in] listener  listener of the CustomerInfo service
 */
extern void
CubiwareMWCustomerInfoRemoveListener(CubiwareMWCustomerInfo self, SvObject listener);

/**
 * Get CustomerInfo data status.
 * @param[in] self      CubiwareMWCustomerInfo handle
 * @return              CubiwareMWCustomerInfoStatus
 */
extern CubiwareMWCustomerInfoStatus
CubiwareMWCustomerInfoGetStatus(CubiwareMWCustomerInfo self);

/**
 * Get error code returned by the Cubiware MW on CustomerInfo data request.
 * @param[in] self      CubiwareMWCustomerInfo handle
 * @return              error code as int
 */
extern int
CubiwareMWCustomerInfoGetErrorCode(CubiwareMWCustomerInfo self);

/**
 * Force CustomerInfo data refresh action.
 * @param[in] self      CubiwareMWCustomerInfo handle
 */
extern void
CubiwareMWCustomerInfoRefresh(CubiwareMWCustomerInfo self);

/**
 * Continuous or single-shot modes can be set using this function.
 * @param[in] self              CubiwareMWCustomerInfo handle
 * @param[in] workContinuously  if true then the refreshTimeSec set in Create function will be used for scheduling
 */
extern void
CubiwareMWCustomerInfoSetWorkingMode(CubiwareMWCustomerInfo self, bool workContinuously);

/**
 * Set customer id to be used in the Cubiware MW calls.
 * @param[in] self              CubiwareMWCustomerInfo handle
 * @param[in] customerId        customer id to be used by CustomerInfo service
 * @return                      -1 on error
 */
extern int
CubiwareMWCustomerInfoSetCustomerId(CubiwareMWCustomerInfo self, SvString customerId);

/**
 * Set new CustomerInfo data.
 * @param[in] self              CubiwareMWCustomerInfo handle
 * @param[in] customerInfo      new CustomerInfo data
 * @return                      -1 on error
 */
extern int
CubiwareMWCustomerInfoSetCustomerInfo(CubiwareMWCustomerInfo self, SvHashTable customerInfo);

/**
 * Return CustomerInfo data.
 * @param[in] self              CubiwareMWCustomerInfo handle
 * @param[out] customerId       customer id as string
 * @param[out] customerInfo     customer info as hashtable
 * @return                      -1 on error
 */
extern int
CubiwareMWCustomerInfoGetCustomerData(CubiwareMWCustomerInfo self, SvString *customerId, SvHashTable *customerInfo);

/** @} */

#endif /* CUBIWAREMWCUSTOMERINFO_H_ */

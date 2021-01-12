/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

/* App/Libraries/QBContentManager/QBContentMgrTransaction.h */

#ifndef QB_CONTENT_MANAGER_TRANSACTION_H_
#define QB_CONTENT_MANAGER_TRANSACTION_H_

/**
 * @file QBContentMgrTransaction.h
 * @brief Content Manager transaction interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdlib.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataProvider/QBRemoteDataRequest.h>


/**
 * @defgroup QBContentMgrTransaction Content manager transaction inteface
 * @ingroup QBContentManagerCore
 * @{
 **/

/**
 * Content manager transaction status.
 **/
typedef struct {
    /** status of the entire transaction */
    QBRemoteDataRequestStatus status;
    /**
     * max total number of requests that will be needed
     * to perform this transaction
     **/
    unsigned int maxRequestsCnt;
    /** number of completed requests */
    unsigned int completedRequestsCnt;
} QBContentMgrTransactionStatus;

/**
 * Content manager transaction interface.
 **/
typedef struct QBContentMgrTransaction_ {
    /**
     * Get transaction status.
     *
     * @param[in] self_     content manager transaction handle
     * @param[out] status   transaction status
     **/
    void (*getStatus)(SvObject self_,
                      QBContentMgrTransactionStatus *const status);

    /**
     * Set transaction listener.
     *
     * @param[in] self_     content manager transaction handle
     * @param[in] listener  content manager transaction listener handle
     * @param[out] errorOut error info
     **/
    void (*setListener)(SvObject self_,
                        SvObject listener,
                        SvErrorInfo *errorOut);

    /**
     * Start transaction.
     *
     * @param[in] self_     content manager transaction handle
     * @param[in] scheduler handle to a scheduler to be used by the transaction
     **/
    void (*start)(SvObject self_, SvScheduler scheduler);

    /**
     * Cancel transaction.
     *
     * @param[in] self_     content manager transaction handle
     **/
    void (*cancel)(SvObject self_);
} *QBContentMgrTransaction;


/**
 * Get runtime type identification object representing
 * content manager transaction interface.
 * @return Content Manager transaction interface identification object
 **/
extern SvInterface
QBContentMgrTransaction_getInterface(void);

/**
 * @}
 **/


/**
 * @defgroup QBContentMgrTransactionListener Content manager transaction listener inteface
 * @ingroup QBContentManagerCore
 * @{
 **/

/**
 * Content manager transaction listener interface.
 **/
typedef struct QBContentMgrTransactionListener_ {
    /**
     * Notify that status of the content manager transaction had changed.
     *
     * @param[in] self_     content manager transaction listener handle
     * @param[in] transaction content manager transaction handle
     * @param[in] status    current transaction status
     **/
    void (*statusChanged)(SvObject self_,
                          SvObject transaction,
                          const QBContentMgrTransactionStatus *const status);
} *QBContentMgrTransactionListener;


/**
 * Get runtime type identification object representing
 * content manager transaction listener interface.
 * @return Content Manager transaction listener interface identification object
 **/
extern SvInterface
QBContentMgrTransactionListener_getInterface(void);

/**
 * @}
 **/


#endif

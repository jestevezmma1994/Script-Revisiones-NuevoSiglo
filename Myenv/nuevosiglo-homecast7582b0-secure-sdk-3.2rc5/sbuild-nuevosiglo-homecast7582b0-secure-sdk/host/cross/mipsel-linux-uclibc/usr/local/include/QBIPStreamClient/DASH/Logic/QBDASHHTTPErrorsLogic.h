/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBDASHHTTPERRORSLOGIC_H_
#define QBDASHHTTPERRORSLOGIC_H_

/**
 * @file QBDASHHTTPErrorsLogic.h
 * @brief MPEG-DASH HTTP errors logic API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <QBIPStreamClient/Common/QBIPStreamManifestPlaybackInfo.h>
#include <stdbool.h>

/**
 * @defgroup QBDASHHTTPErrorsLogic MPEG-DASH HTTP errors logic
 * @{
 **/

/**
 * Specifies HTTP errors categories
 **/
typedef enum {
    QBDASHHTTPErrorCategory_unknown,            ///< unknown HTTP error category
    QBDASHHTTPErrorCategory_authentication,     ///< problems caused by geoblocking, expired token etc.
    QBDASHHTTPErrorCategory_configuration,      ///< problems caused by network configuration
    QBDASHHTTPErrorCategory_missingSegments,    ///< problems caused by missing segments
    QBDASHHTTPErrorCategory_network,            ///< problems caused by network errors
    QBDASHHTTPErrorCategory_serverOverloaded,   ///< problems with CDN
    QBDASHHTTPErrorCategory_miscellaneous       ///< other errors
} QBDASHHTTPErrorCategory;

/**
 * Provides recovery options for different types of HTTP errors (based on ETSI TS 103 285 V.1.1.1 documentation).
 * HTTP errors are divided in categories for which each we have defined maximum number of retries. New reported HTTP error
 * is assigned to error category (retry counter is reseted for HTTP error from new category).
 * By calling method @ref QBDASHHTTPErrorsLogicShouldRetryDownload we are checking if retry counter is smaller then maximum number of retries
 * for given error category.
 * Retry counter is updated by calling method @ref QBDASHHTTPErrorsLogicUpdateRetryCounter.
 * @class QBDASHManifestLogic
 * @extends SvObject
 **/
typedef struct QBDASHHTTPErrorsLogic_ *QBDASHHTTPErrorsLogic;

/**
 * Get runtime type identification object representing
 * type of MPEG-DASH HTTP errors logic class.
 *
 * @return MPEG-DASH HTTP errors logic class
 **/
SvType
QBDASHHTTPErrorsLogic_getType(void);

/**
 * Create MPEG-DASH HTTP errors logic.
 *
 * @param[out] errorOut error info
 * @return              created logic, @c NULL in case of error
 **/
QBDASHHTTPErrorsLogic
QBDASHHTTPErrorsLogicCreate(SvErrorInfo *errorOut);

/**
 * Set media presentation type.
 *
 * @param[in]  self                  HTTP error logic handle
 * @param[in]  mediaPresentationType media presentation type
 * @param[out] errorOut              error info
 **/
void
QBDASHHTTPErrorsLogicSetMediaPresentationType(QBDASHHTTPErrorsLogic self,
                                              QBIPStreamMediaPresentationType mediaPresentationType,
                                              SvErrorInfo *errorOut);

/**
 * Report HTTP error to logic and update retry counters.
 *
 * @param[in]  self          HTTP error logic handle
 * @param[in]  httpErrorCode HTTP error code
 * @param[out] errorOut      error info
 **/
void
QBDASHHTTPErrorsLogicReportHTTPError(QBDASHHTTPErrorsLogic self,
                                     int httpErrorCode,
                                     SvErrorInfo *errorOut);

/**
 * Increase retry counter.
 *
 * @param[in] self      HTTP error logic handle
 * @param[out] errorOut error info
 **/
void
QBDASHHTTPErrorsLogicUpdateRetryCounter(QBDASHHTTPErrorsLogic self,
                                        SvErrorInfo *errorOut);

/**
 * Reset retry counter.
 *
 * @param[in]  self     HTTP error logic handle
 * @param[out] errorOut error info
 **/
void
QBDASHHTTPErrorsLogicResetRetryCounter(QBDASHHTTPErrorsLogic self,
                                       SvErrorInfo *errorOut);

/**
 * Check if is required to start the download again.
 *
 * @param[in] self HTTP error logic handle
 * @return         @c true if re-download is required, otherwise @c false
 **/
bool
QBDASHHTTPErrorsLogicShouldRetryDownload(QBDASHHTTPErrorsLogic self);

/**
 * Get category of last reported HTTP error.
 *
 * @param[in] self HTTP error logic handle
 * @return         category of HTTP error
 **/
QBDASHHTTPErrorCategory
QBDASHHTTPErrorsLogicGetErrorCategory(QBDASHHTTPErrorsLogic self);

/**
 * @}
 **/

#endif /* QBDASHHTTPERRORSLOGIC_H_ */

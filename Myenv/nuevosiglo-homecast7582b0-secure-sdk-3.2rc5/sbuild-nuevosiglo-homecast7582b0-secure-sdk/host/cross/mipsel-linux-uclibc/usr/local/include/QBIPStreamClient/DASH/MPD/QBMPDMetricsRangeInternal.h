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

#ifndef QBMPDMETRICSRANGEINTERNAL_H_
#define QBMPDMETRICSRANGEINTERNAL_H_

/**
 * @file QBMPDMetricsRangeInternal.h
 * @brief Metrics Range element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDMetricsRange.h>

/**
 * @addtogroup QBMPDMetricsRange
 * @{
 **/

/**
 * Create Metrics Range element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Metrics Range element, @c NULL in case of error
 **/
extern QBMPDMetricsRange
QBMPDMetricsRangeCreate(SvErrorInfo *errorOut);

/**
 * Set start time for Metrics Range element.
 *
 * @param[in]  self      Metrics Range element handle
 * @param[in]  startTime start time
 * @param[out] errorOut  error info
 **/
extern void
QBMPDMetricsRangeSetStartTime(QBMPDMetricsRange self,
                              SvTime startTime,
                              SvErrorInfo *errorOut);

/**
 * Set duration for Metrics Range element.
 *
 * @param[in]  self      Metrics Range element handle
 * @param[in]  duration  duration
 * @param[out] errorOut  error info
 **/
extern void
QBMPDMetricsRangeSetDuration(QBMPDMetricsRange self,
                             SvTime duration,
                             SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDMETRICSRANGEINTERNAL_H_ */

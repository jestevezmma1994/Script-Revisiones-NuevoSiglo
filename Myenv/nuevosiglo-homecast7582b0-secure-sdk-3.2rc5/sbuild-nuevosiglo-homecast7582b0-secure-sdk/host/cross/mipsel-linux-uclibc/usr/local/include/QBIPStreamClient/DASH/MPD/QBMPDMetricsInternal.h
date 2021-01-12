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

#ifndef QBMPDMETRICSINTERNAL_H_
#define QBMPDMETRICSINTERNAL_H_

/**
 * @file QBMPDMetricsInternal.h
 * @brief Metrics element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDMetricsRange.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDDescriptor.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDMetrics.h>

/**
 * @addtogroup QBMPDMetrics
 * @{
 **/

/**
 * Create Metrics element.
 *
 * @param[in]  metrics  list of MPEG-DASH Metric keys
 * @param[out] errorOut error info
 * @return              new instance of Metrics element, @c NULL in case of error
 **/
extern QBMPDMetrics
QBMPDMetricsCreate(SvString metrics,
                   SvErrorInfo *errorOut);

/**
 * Add range element to Metrics element.
 *
 * @param[in]  self     Metrics element handle
 * @param[in]  range    Metrics range
 * @param[out] errorOut error info
 **/
extern void
QBMPDMetricsAddRange(QBMPDMetrics self,
                     QBMPDMetricsRange range,
                     SvErrorInfo *errorOut);
/**
 * Add reporting element to Metrics element.
 *
 * @param[in]  self      Metrics element handle
 * @param[in]  reporting reporting method and formats
 * @param[out] errorOut  error info
 **/
extern void
QBMPDMetricsAddReporting(QBMPDMetrics self,
                         QBMPDDescriptor reporting,
                         SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDMETRICSINTERNAL_H_ */

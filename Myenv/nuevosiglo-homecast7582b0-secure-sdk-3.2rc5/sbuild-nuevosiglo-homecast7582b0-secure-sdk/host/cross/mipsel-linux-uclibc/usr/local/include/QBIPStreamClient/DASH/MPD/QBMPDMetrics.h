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

#ifndef QBMPDMETRICS_H_
#define QBMPDMETRICS_H_

/**
 * @file QBMPDMetrics.h
 * @brief Metrics element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>

/**
 * @defgroup QBMPDMetrics Metrics element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Metrics element class.
 * @class QBMPDMetrics
 * @extends SvObject
 **/
typedef struct QBMPDMetrics_ *QBMPDMetrics;

/**
 * Get runtime type identification object representing
 * type of Metrics element class.
 *
 * @return Metrics element class
 **/
extern SvType
QBMPDMetrics_getType(void);

/**
 * Get metrics of Metrics element.
 *
 * @param[in] self Metrics element handle
 * @return         list of MPEG-DASH Metrics keys
 **/
extern SvString
QBMPDMetricsGetMetrics(QBMPDMetrics self);

/**
 * Get array of Metrics Range elements of Metrics element.
 *
 * @param[in] self      Metrics element handle
 * @return              array of Metrics Range elements, @c NULL in case of error
 **/
extern SvArray
QBMPDMetricsGetRanges(QBMPDMetrics self);

/**
 * Get array of reporting descriptor elements of Metrics element.
 *
 * @param[in] self      Metrics element handle
 * @return              array of reporting descriptor elements, @c NULL in case of error
 **/
extern SvArray
QBMPDMetricsGetReportings(QBMPDMetrics self);

/**
 * @}
 **/

#endif /* QBMPDMETRICS_H_ */

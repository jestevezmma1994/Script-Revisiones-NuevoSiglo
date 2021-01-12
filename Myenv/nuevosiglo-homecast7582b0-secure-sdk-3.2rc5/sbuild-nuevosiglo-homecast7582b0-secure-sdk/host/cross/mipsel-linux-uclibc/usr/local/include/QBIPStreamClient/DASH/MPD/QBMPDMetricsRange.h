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

#ifndef QBMPDMETRICSRANGE_H_
#define QBMPDMETRICSRANGE_H_

/**
 * @file QBMPDMetricsRange.h
 * @brief Metrics Range element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBMPDMetricsRange Metrics Range element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Metrics Range element class.
 * @class QBMPDMetricsRange
 * @extends SvObject
 **/
typedef struct QBMPDMetricsRange_ *QBMPDMetricsRange;

/**
 * Get runtime type identification object representing
 * type of Metrics Range element class.
 *
 * @return Metrics Range element class
 **/
extern SvType
QBMPDMetricsRange_getType(void);

/**
 * Get start time of Metrics Range element.
 *
 * @param[in] self Metrics Range element handle
 * @return         start time
 **/
extern SvTime
QBMPDMetricsRangeGetStartTime(QBMPDMetricsRange self);

/**
 * Get duration of Metrics Range element.
 *
 * @param[in] self Metrics Range element handle
 * @return         duration
 **/
extern SvTime
QBMPDMetricsRangeGetDuration(QBMPDMetricsRange self);

/**
 * @}
 **/

#endif /* QBMPDMETRICSRANGE_H_ */

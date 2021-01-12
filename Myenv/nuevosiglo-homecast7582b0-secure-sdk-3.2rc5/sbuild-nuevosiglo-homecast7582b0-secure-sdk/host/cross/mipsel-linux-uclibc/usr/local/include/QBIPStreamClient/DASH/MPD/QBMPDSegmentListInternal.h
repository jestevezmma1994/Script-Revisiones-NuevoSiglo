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

#ifndef QBMPDSEGMENTLISTINTERNAL_H_
#define QBMPDSEGMENTLISTINTERNAL_H_

/**
 * @file QBMPDSegmentListInternal.h
 * @brief Segment List element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDXlink.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentURL.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentList.h>

/**
 * @addtogroup QBMPDSegmentList
 * @{
 **/

/**
 * Create Segment List element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Segment List element, @c NULL in case of error
 **/
extern QBMPDSegmentList
QBMPDSegmentListCreate(SvErrorInfo *errorOut);

/**
 * Set xlink for Segment List element.
 *
 * @param[in]  self     Segment List element handle
 * @param[in]  xlink    Xlink element
 * @param[out] errorOut error info
 **/
extern void
QBMPDSegmentListSetXlink(QBMPDSegmentList self,
                         QBMPDXlink xlink,
                         SvErrorInfo *errorOut);

/**
 * Add Media Segment URL for Segment List element.
 *
 * @param[in]  self       Segment List element handle
 * @param[in]  segmentURL Media Segment URL
 * @param[out] errorOut   error info
 **/
extern void
QBMPDSegmentListAddSegmentURL(QBMPDSegmentList self,
                              QBMPDSegmentURL segmentURL,
                              SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTLISTINTERNAL_H_ */

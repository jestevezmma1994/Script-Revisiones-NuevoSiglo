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

#ifndef QBMPDSEGMENTURLINTERNAL_H_
#define QBMPDSEGMENTURLINTERNAL_H_

/**
 * @file QBMPDSegmentURLInternal.h
 * @brief Segment URL element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentURL.h>

/**
 * @addtogroup QBMPDSegmentURL
 * @{
 **/

/**
 * Create Segment URL element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Segment URL element, @c NULL in case of error
 **/
extern QBMPDSegmentURL
QBMPDSegmentURLCreate(SvErrorInfo *errorOut);

/**
 * Set URL to Media Segment for Segment URL element.
 *
 * @param[in]  self     Segment URL element handle
 * @param[in]  media    URL for Media Segment (it shall be formatted as absolute URI or relative reference)
 * @param[out] errorOut error info
 **/
extern void
QBMPDSegmentURLSetMedia(QBMPDSegmentURL self,
                        SvURI media,
                        SvErrorInfo *errorOut);

/**
 * Set byte range for Segment URL element.
 *
 * @param[in]  self       Segment URL element handle
 * @param[in]  mediaRange bytes range corresponding to the Media Segment
 * @param[out] errorOut   error info
 **/
extern void
QBMPDSegmentURLSetMediaRange(QBMPDSegmentURL self,
                             SvString mediaRange,
                             SvErrorInfo *errorOut);

/**
 * Set URL to Index Segment for Segment URL element.
 *
 * @param[in]  self     Segment URL element handle
 * @param[in]  index    URL for Index Segment (it shall be formatted as absolute URI or relative reference)
 * @param[out] errorOut error info
 **/
extern void
QBMPDSegmentURLSetIndex(QBMPDSegmentURL self,
                        SvURI index,
                        SvErrorInfo *errorOut);

/**
 * Set byte range corresponding to the Index Segment for Segment URL element.
 *
 * @param[in]  self         Segment URL element handle
 * @param[in]  indexRange   byte range for Index Segment
 * @param[out] errorOut     error info
 **/
extern void
QBMPDSegmentURLSetIndexRange(QBMPDSegmentURL self,
                             SvString indexRange,
                             SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTURLINTERNAL_H_ */

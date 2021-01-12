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

#ifndef QBMPDSEGMENTTEMPLATEINTERNAL_H_
#define QBMPDSEGMENTTEMPLATEINTERNAL_H_

/**
 * @file QBMPDSegmentTemplateInternal.h
 * @brief Segment Template element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTemplate.h>

/**
 * @addtogroup QBMPDSegmentTemplate
 * @{
 **/

/**
 * Create Segment Template element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Segment Template, @c NULL in case of error
 **/
extern QBMPDSegmentTemplate
QBMPDSegmentTemplateCreate(SvErrorInfo *errorOut);

/**
 * Set media for Segment Template element.
 *
 * @param[in]  self     Segment Template element handle
 * @param[in]  media    template to create Media Segment List
 * @param[out] errorOut error info
 **/
extern void
QBMPDSegmentTemplateSetMedia(QBMPDSegmentTemplate self,
                             SvString media,
                             SvErrorInfo *errorOut);

/**
 * Set index for Segment Template element.
 *
 * @param[in]  self     Segment Template element handle
 * @param[in]  index    template to create Index Segment List
 * @param[out] errorOut error info
 **/
extern void
QBMPDSegmentTemplateSetIndex(QBMPDSegmentTemplate self,
                             SvString index,
                             SvErrorInfo *errorOut);

/**
 * Set initialization for Segment Template element.
 *
 * @param[in]  self             Segment Template element handle
 * @param[in]  initialization   template to create Initialization Segment
 * @param[out] errorOut         error info
 **/
extern void
QBMPDSegmentTemplateSetInitialization(QBMPDSegmentTemplate self,
                                      SvString initialization,
                                      SvErrorInfo *errorOut);

/**
 * Set bitstream switching for Segment Template element.
 *
 * @param[in]  self               Segment Template element handle
 * @param[in]  bitstreamSwitching template to create Bitstream Switching Segment
 * @param[out] errorOut           error info
 **/
extern void
QBMPDSegmentTemplateSetBitstreamSwitching(QBMPDSegmentTemplate self,
                                          SvString bitstreamSwitching,
                                          SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTTEMPLATEINTERNAL_H_ */

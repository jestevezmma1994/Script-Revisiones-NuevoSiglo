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

#ifndef QBMPDSEGMENTLIST_H_
#define QBMPDSEGMENTLIST_H_

/**
 * @file QBMPDSegmentList.h
 * @brief Segment List element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDXlink.h>

/**
 * @defgroup QBMPDSegmentList Segment List element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Segment List element class.
 * @class QBMPDSegmentList
 * @extends QBMPDMultipleSegmentBase
 **/
typedef struct QBMPDSegmentList_ *QBMPDSegmentList;

/**
 * Get runtime type identification object representing
 * type of Segment List element class.
 *
 * @return Segment List element class
 **/
extern SvType
QBMPDSegmentList_getType(void);

/**
 * Get xlink of Segment List element.
 *
 * @param[in] self Segment List element handle
 * @return         Xlink element, @c NULL in case of error
 **/
extern QBMPDXlink
QBMPDSegmentListGetXlink(QBMPDSegmentList self);

/**
 * Get array of Media Segment URLs of Segment List element.
 *
 * @param[in]  self     Segment List element handle
 * @return              array of Media Segment URLs, @c NULL in case of error
 **/
extern SvArray
QBMPDSegmentListGetSegmentURLs(QBMPDSegmentList self);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTLIST_H_ */

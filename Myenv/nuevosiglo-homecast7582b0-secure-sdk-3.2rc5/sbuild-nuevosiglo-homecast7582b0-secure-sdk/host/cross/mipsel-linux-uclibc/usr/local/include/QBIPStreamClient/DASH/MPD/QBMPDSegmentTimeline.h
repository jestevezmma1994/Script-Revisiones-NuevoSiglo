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

#ifndef QBMPDSEGMENTTIMELINE_H_
#define QBMPDSEGMENTTIMELINE_H_

/**
 * @file QBMPDSegmentTimeline.h
 * @brief Segment Timeline element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDTimeline.h>
#include <QBAppKit/QBOpaqueValueArray.h>

/**
 * @defgroup QBMPDSegmentTimeline Segment Timeline element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Segment Timeline element class.
 * @class QBMPDSegmentTimeline
 * @extends SvObject
 **/
typedef struct QBMPDSegmentTimeline_ *QBMPDSegmentTimeline;

/**
 * Get runtime type identification object representing
 * type of Segment Timeline element class.
 *
 * @return Segment Timeline element class
 **/
extern SvType
QBMPDSegmentTimeline_getType(void);

/**
 * Get array of Timeline elements of Segment Timeline element.
 *
 * @param[in]  self     Segment Timeline element handle
 * @return              array of Timeline elements, @c NULL in case of error
 **/
extern QBOpaqueValueArray
QBMPDSegmentTimelineGetTimelines(QBMPDSegmentTimeline self);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTTIMELINE_H_ */

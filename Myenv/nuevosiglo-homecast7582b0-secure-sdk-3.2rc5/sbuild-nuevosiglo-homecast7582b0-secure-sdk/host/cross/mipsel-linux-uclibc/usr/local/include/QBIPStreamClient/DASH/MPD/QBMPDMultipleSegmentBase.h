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

#ifndef QBMPDMULTIPLESEGMENTBASE_H_
#define QBMPDMULTIPLESEGMENTBASE_H_

/**
 * @file QBMPDMultipleSegmentBase.h
 * @brief Multiple Segment Base element class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTimeline.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDURL.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentBase.h>

/**
 * @defgroup QBMPDMultipleSegmentBase Multiple Segment Base element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Multiple Segment Base element class.
 * @class QBMPDMultipleSegmentBase
 * @extends QBMPDSegmentBase
 **/
typedef struct QBMPDMultipleSegmentBase_ *QBMPDMultipleSegmentBase;

/**
 * Get runtime type identification object representing
 * type of Multiple Segment Base element class.
 *
 * @return Multiple Segment Base element class
 **/
extern SvType
QBMPDMultipleSegmentBase_getType(void);

/**
 * Get duration of Multiple Segment Base element.
 *
 * @param[in] self Multiple Segment Base element handle
 * @return         duration in @c timescale units
 **/
extern uint64_t
QBMPDMultipleSegmentBaseGetDuration(QBMPDMultipleSegmentBase self);

/**
 * Get start number of Multiple Segment Base element.
 *
 * @param[in] self Multiple Segment Base element handle
 * @return         start number
 **/
extern unsigned int
QBMPDMultipleSegmentBaseGetStartNumber(QBMPDMultipleSegmentBase self);

/**
 * Get Segment Timeline element of Multiple Segment Base element.
 *
 * @param[in] self Multiple Segment Base element handle
 * @return         Segment Timeline element, @c NULL in case of error
 **/
extern QBMPDSegmentTimeline
QBMPDMultipleSegmentBaseGetSegmentTimeline(QBMPDMultipleSegmentBase self);

/**
 * Get URL including byte range for Bitstream Switching Segment of Multiple Segment Base element.
 *
 * @param[in] self Multiple Segment Base element handle
 * @return         URL including byte range for Bitstream Switching Segment, @c NULL in case of error
 **/
extern QBMPDURL
QBMPDMultipleSegmentBaseGetBitstreamSwitching(QBMPDMultipleSegmentBase self);

/**
 * @}
 **/

#endif /* QBMPDMULTIPLESEGMENTBASE_H_ */

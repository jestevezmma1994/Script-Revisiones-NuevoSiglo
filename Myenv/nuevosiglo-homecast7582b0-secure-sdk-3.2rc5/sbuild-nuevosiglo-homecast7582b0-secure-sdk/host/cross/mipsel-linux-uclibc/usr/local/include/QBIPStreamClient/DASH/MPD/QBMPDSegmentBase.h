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

#ifndef QBMPDSEGMENTBASE_H_
#define QBMPDSEGMENTBASE_H_

/**
 * @file QBMPDSegmentBase.h
 * @brief Segment Base element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDURL.h>
#include <stdbool.h>

/**
 * @defgroup QBMPDSegmentBase Segment Base element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Segment Base element class.
 * @class QBMPDSegmentBase
 * @extends SvObject
 **/
typedef struct QBMPDSegmentBase_ *QBMPDSegmentBase;

/**
 * Get runtime type identification object representing
 * type of Segment Base element class.
 *
 * @return Segment Base element class
 **/
extern SvType
QBMPDSegmentBase_getType(void);

/**
 * Get timescale of Segment Base element.
 *
 * @param[in] self Segment Base element handle
 * @return         timescale
 **/
extern uint64_t
QBMPDSegmentBaseGetTimescale(QBMPDSegmentBase self);

/**
 * Get presentation timescale offset of Segment Base element.
 *
 * @param[in] self Segment Base element handle
 * @return         presentation time offset
 **/
extern uint64_t
QBMPDSegmentBaseGetPresentationTimeOffset(QBMPDSegmentBase self);

/**
 * Get time shifting buffer of Segment Base element.
 *
 * @param[in] self Segment Base element handle
 * @return         time shifting buffer
 **/
extern SvTime
QBMPDSegmentBaseGetTimeShiftBufferDepth(QBMPDSegmentBase self);

/**
 * Get index range of Segment Base element.
 *
 * @param[in] self Segment Base element handle
 * @return         index range, @c NULL in case of error
 **/
extern SvString
QBMPDSegmentBaseGetIndexRange(QBMPDSegmentBase self);

/**
 * Get index range excat of Segment Base element.
 *
 * @param[in]  self     Segment Base element handle
 * @return              index range excat
 **/
extern bool
QBMPDSegmentBaseGetIndexRangeExact(QBMPDSegmentBase self);
/**
 * Get availability time offset of Segment Base element.
 *
 * @param[in] self Segment Base element handle
 * @return         availability time offset
 **/
extern double
QBMPDSegmentBaseGetAvailabilityTimeOffset(QBMPDSegmentBase self);

/**
 * Get availability time complete of Segment Base element.
 *
 * @param[in]  self     Segment Base element handle
 * @return              available time complete
 **/
extern bool
QBMPDSegmentBaseGetAvailabilityTimeComplete(QBMPDSegmentBase self);
/**
 * Get initialization of Segment Base element.
 *
 * @param[in] self Segment Base element handle
 * @return         URL including possible byte range for Initialization Segment, @c NULL in case of error
 **/
extern QBMPDURL
QBMPDSegmentBaseGetInitialization(QBMPDSegmentBase self);

/**
 * Get representation index of Segment Base element.
 *
 * @param[in] self Segment Base element handle
 * @return         URL including possible byte range for Representation Index Segment, @c NULL in case of error
 **/
extern QBMPDURL
QBMPDSegmentBaseGetRepresentationIndex(QBMPDSegmentBase self);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTBASE_H_ */

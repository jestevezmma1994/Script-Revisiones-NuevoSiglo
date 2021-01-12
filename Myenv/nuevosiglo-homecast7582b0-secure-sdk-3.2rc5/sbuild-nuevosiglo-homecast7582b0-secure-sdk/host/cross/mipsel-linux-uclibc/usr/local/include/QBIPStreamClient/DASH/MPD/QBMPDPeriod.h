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

#ifndef QBMPDPERIOD_H_
#define QBMPDPERIOD_H_

/**
 * @file QBMPDPeriod.h
 * @brief Period element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDDescriptor.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentBase.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentList.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTemplate.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDXlink.h>
#include <stdbool.h>

/**
 * @defgroup QBMPDPeriod Period element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Period element class.
 * @class QBMPDPeriod
 * @extends SvObject
 **/
typedef struct QBMPDPeriod_ *QBMPDPeriod;

/**
 * Get runtime type identification object representing
 * type of Period element class.
 *
 * @return Period element class
 **/
extern SvType
QBMPDPeriod_getType(void);

/**
 * Get xlink of Period element.
 *
 * @param[in] self Period element handle
 * @return         Xlink element, @c NULL in case of error
 **/
extern QBMPDXlink
QBMPDPeriodGetXlink(QBMPDPeriod self);

/**
 * Get identifier of Period element.
 *
 * @param[in] self Period element handle
 * @return         identifier, @c NULL in case of error
 **/
extern SvString
QBMPDPeriodGetId(QBMPDPeriod self);

/**
 * Get start time of Period element.
 *
 * @param[in]   self        Period element handle
 * @param[out]  startOut    start time
 * @return      @c true if period contains start time, @c false otherwise
 *
 **/
extern bool
QBMPDPeriodGetStart(QBMPDPeriod self, SvTime* startOut);

/**
 * Get duration of Period element.
 *
 * @param[in]   self        Period element handle
 * @param[out]  durationOut duration
 * @return      @c true if period contains duration, @c false otherwise
 **/
extern bool
QBMPDPeriodGetDuration(QBMPDPeriod self, SvTime* durationOut);

/**
 * Get bitstream switching attribute of Period element.
 *
 * @param[in]  self     Period element handle
 * @return              bitstream switching attribute
 **/
extern bool
QBMPDPeriodGetBitstreamSwitching(QBMPDPeriod self);

/**
 * Get array of Base URL elements of Period element.
 *
 * @param[in]  self     Period element handle
 * @return              array of Base URL elements, @c NULL in case of error
 **/
extern SvArray
QBMPDPeriodGetBaseURLs(QBMPDPeriod self);

/**
 * Get Segment Base element of Period element.
 *
 * @param[in] self Period element handle
 * @return         Segment Base element, @c NULL in case of error
 **/
extern QBMPDSegmentBase
QBMPDPeriodGetSegmentBase(QBMPDPeriod self);

/**
 * Get Segment List element of Period element.
 *
 * @param[in] self Period element handle
 * @return         Segment List element, @c NULL in case of error
 **/
extern QBMPDSegmentList
QBMPDPeriodGetSegmentList(QBMPDPeriod self);

/**
 * Get Segment Template element of Period element.
 *
 * @param[in] self Period element handle
 * @return         Segment Template element, @c NULL in case of error
 **/
extern QBMPDSegmentTemplate
QBMPDPeriodGetSegmentTemplate(QBMPDPeriod self);

/**
 * Get asset identifier descriptor of Period element.
 *
 * @param[in] self Period element handle
 * @return         asset identifier descriptor
 **/
extern QBMPDDescriptor
QBMPDPeriodGetAssetIdentifier(QBMPDPeriod self);

/**
 * Get array of Event Stream elements of Period element.
 *
 * @param[in]  self     Period element handle
 * @return              array of Event Stream, @c NULL in case of error
 **/
extern SvArray
QBMPDPeriodGetEventStreams(QBMPDPeriod self);

/**
 * Get array of Adaptation Set elements of Period element.
 *
 * @param[in]  self     Period element handle
 * @return              array of Adaptation Set elements, @c NULL in case of error
 **/
extern SvArray
QBMPDPeriodGetAdaptationSets(QBMPDPeriod self);

/**
 * Get array of Subset elements of Period element.
 *
 * @param[in]  self     Period element handle
 * @return              array of Subset elements, @c NULL in case of error
 **/
extern SvArray
QBMPDPeriodGetSubsets(QBMPDPeriod self);

/**
 * @}
 **/

#endif /* QBMPDPERIOD_H_ */

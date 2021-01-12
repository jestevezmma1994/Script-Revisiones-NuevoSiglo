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

#ifndef QBMPDELEMENT_H_
#define QBMPDELEMENT_H_

/**
 * @file QBMPDElement.h
 * @brief Media Presentation Description element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>

/**
 * Specifies the type of the Media Presentation
 **/
typedef enum {
    QBMPDElementPresentationType_unknown = -1, ///< special marker when type is unknown
    QBMPDElementPresentationType_static,       ///< for static Media Presentation
    QBMPDElementPresentationType_dynamic       ///< for dynamic Media Presentation
} QBMPDElementPresentationType;

/**
 * @defgroup QBMPDElement Media Presentation Description element class
 * @{
 **/

/**
 * Media Presentation Description element class.
 * @class QBMPDElement
 * @extends SvObject
 **/
typedef struct QBMPDElement_ *QBMPDElement;

/**
 * Get runtime type identification object representing
 * type of Media Presentation Description element class.
 *
 * @return Media Presentation Description element class
 **/
extern SvType
QBMPDElement_getType(void);

/**
 * Get identifier of MPD element.
 *
 * @param[in]  self     MPD element handle
 * @return              identifier, @c NULL in case of error
 **/
extern SvString
QBMPDElementGetId(QBMPDElement self);

/**
 * Get profiles of MPD element.
 *
 * @param[in]  self     MPD element handle
 * @return              profiles, @c NULL in case of error
 **/
extern SvString
QBMPDElementGetProfiles(QBMPDElement self);

/**
 * Get type of MPD element.
 *
 * @param[in]  self     MPD element handle
 * @return              type of MPD, @c QBMPDElementPresentationType_unknown in case of error
 **/
extern QBMPDElementPresentationType
QBMPDElementGetPresentationType(QBMPDElement self);

/**
 * Get availability start time of MPD element.
 *
 * @param[in]  self                     MPD element handle
 * @param[out] availabilityStartTimeOut availability start time
 * @return                              @c true if the manifest had the availabilityStartTime, @c false otherwise
 **/
extern bool
QBMPDElementGetAvailabilityStartTime(QBMPDElement self, SvTime* availabilityStartTimeOut);

/**
 * Get availability end time of MPD element.
 *
 * @param[in]  self                     MPD element handle
 * @param[out] availabilityEndTimeOut   availability end time
 * @return                              @c true if the manifest had the availabilityEndTime, @c false otherwise
 **/
extern bool
QBMPDElementGetAvailabilityEndTime(QBMPDElement self, SvTime* availabilityEndTimeOut);

/**
 * Get publish time of MPD element.
 *
 * @param[in]  self             MPD element handle
 * @param[out] publishTimeOut   publish time
 * @return                      @c true if the manifest had the publishTime, @c false otherwise
 **/
extern bool
QBMPDElementGetPublishTime(QBMPDElement self, SvTime* publishTimeOut);

/**
 * Get media presentation duration of MPD element.
 *
 * @param[in]  self                         MPD element handle
 * @param[out] mediaPresentationDurationOut media presentation duration
 * @return                                  @c true if the manifest had the mediaPresentationDuration, @c false otherwise
 **/
extern bool
QBMPDElementGetMediaPresentationDuration(QBMPDElement self, SvTime* mediaPresentationDurationOut);

/**
 * Get minimum update period of MPD element.
 *
 * @param[in]  self                     MPD element handle
 * @param[out] minimumUpdatePeriodOut   minimumUpdatePeriod to be filled
 * @return                              @c true if the manifest had the minimumUpdatePeriod, @c false otherwise
 **/
extern bool
QBMPDElementGetMinimumUpdatePeriod(QBMPDElement self, SvTime* minimumUpdatePeriodOut);

/**
 * Get minimum buffer time of MPD element.
 *
 * @param[in]  self                 MPD element handle
 * @param[out] minBufferTimeOut     minimum buffer time
 * @return                          @c true if the manifest had the minBufferTime, @c false otherwise
 **/
extern bool
QBMPDElementGetMinBufferTime(QBMPDElement self, SvTime* minBufferTimeOut);

/**
 * Get time shift buffer depth of MPD element.
 *
 * @param[in]  self                     MPD element handle
 * @param[out] timeShiftBufferDepthOut  time shift buffer depth
 * @return                              @c true if the manifest had the timeShiftBufferDepth, @c false otherwise
 **/
extern bool
QBMPDElementGetTimeShiftBufferDepth(QBMPDElement self, SvTime* timeShiftBufferDepthOut);

/**
 * Get suggest presentation delay of MPD element.
 *
 * @param[in]  self                         MPD element handle
 * @param[out] suggestPresentationDelayOut  suggest presentation delay
 * @return                                  @c true if the manifest had the suggestPresentationDelay, @c false otherwise
 **/
extern bool
QBMPDElementGetSuggestPresentationDelay(QBMPDElement self, SvTime* suggestPresentationDelayOut);

/**
 * Get max segment duration of MPD element.
 *
 * @param[in]  self                     MPD element handle
 * @param[out] maxSegmentDurationOut    max segment duration
 * @return                              @c true if the manifest had the maxSegmentDuration, @c false otherwise
 **/
extern bool
QBMPDElementGetMaxSegmentDuration(QBMPDElement self, SvTime* maxSegmentDurationOut);

/**
 * Get max subsegment duration of MPD element.
 *
 * @param[in]  self                     MPD element handle
 * @param[out] maxSubsegmentDurationOut max subsegment duration
 * @return                              @c true if the manifest had the maxSubsegmentDuration, @c false otherwise
 **/
extern bool
QBMPDElementGetMaxSubsegmentDuration(QBMPDElement self, SvTime* maxSubsegmentDurationOut);

/**
 * Get array of program information elements of MPD element.
 *
 * @param[in]  self     MPD element handle
 * @return              array of program information elements, @c NULL in case of error
 **/
extern SvArray
QBMPDElementGetProgramInformations(QBMPDElement self);

/**
 * Get array of base URL elements of MPD element.
 *
 * @param[in]  self     MPD element handle
 * @return              array of program base URL elements, @c NULL in case of error
 **/
extern SvArray
QBMPDElementGetBaseURLs(QBMPDElement self);

/**
 * Get array of location elements of MPD element.
 *
 * @param[in]  self     MPD element handle
 * @return              array of location elements, @c NULL in case of error
 **/
extern SvArray
QBMPDElementGetLocations(QBMPDElement self);

/**
 * Get array of period elements of MPD element.
 *
 * @param[in]  self     MPD element handle
 * @return              array of location elements, @c NULL in case of error
 **/
extern SvArray
QBMPDElementGetPeriods(QBMPDElement self);

/**
 * Get array of metrics elements of MPD element.
 *
 * @param[in]  self     MPD element handle
 * @return              array of metrics elements, @c NULL in case of error
 **/
extern SvArray
QBMPDElementGetMetrics(QBMPDElement self);

/**
 * @}
 **/

#endif /* QBMPDELEMENT_H_ */

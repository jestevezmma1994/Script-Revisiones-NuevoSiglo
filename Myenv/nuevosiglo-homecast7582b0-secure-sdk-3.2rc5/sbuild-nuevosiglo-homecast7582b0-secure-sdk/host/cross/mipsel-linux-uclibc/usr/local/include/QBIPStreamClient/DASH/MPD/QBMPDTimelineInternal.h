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

#ifndef QBMPDTIMELINEINTERNAL_H_
#define QBMPDTIMELINEINTERNAL_H_

/**
 * @file QBMPDTimelineInternal.h
 * @brief Timeline element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDTimeline.h>

/**
 * @addtogroup QBMPDTimeline
 * @{
 **/

/**
 * Timeline element class.
 **/
struct QBMPDTimeline_ {
    uint64_t startTime;      ///< segment start time in timescale units
    uint64_t duration;       ///< segment duration in timescale units
    int repeatCount;         ///< number of following segments with the same duration
};

/**
 * Timeline element class.
 * @class QBMPDTimelineObject
 * @extends SvObject
 **/
typedef struct QBMPDTimelineObject_ {
    struct SvObject_ super_; ///< super class
    QBMPDTimeline data;      ///< timeline element data
} *QBMPDTimelineObject;

/**
 * Get runtime type identification object representing
 * type of Timeline element class.
 *
 * @return Timeline element class
 **/
extern SvType
QBMPDTimelineObject_getType(void);

/**
 * Create Timeline element.
 *
 * @param[in]  duration segment duration in timescale units
 * @param[out] errorOut error info
 * @return              new instance of Timeline element, @c NULL in case of error
 **/
extern QBMPDTimeline
QBMPDTimelineCreate(uint64_t duration,
                    SvErrorInfo *errorOut);

/**
 * Create Timeline element as a SvObject.
 *
 * @param[in]  duration segment duration in timescale units
 * @param[out] errorOut error info
 * @return              new instance of Timeline element, @c NULL in case of error
 **/
extern QBMPDTimelineObject
QBMPDTimelineObjectCreate(uint64_t duration,
                          SvErrorInfo *errorOut);

/**
 * Set segment start time in timescale units.
 *
 * @param[in]  self      Timeline element handle
 * @param[in]  startTime segment start time in timescale units
 * @param[out] errorOut  error info
 **/
extern void
QBMPDTimelineSetStartTime(QBMPDTimeline self,
                          uint64_t startTime,
                          SvErrorInfo *errorOut);

/**
 * Set segment repeat count.
 *
 * @param[in]  self        Timeline element handle
 * @param[in]  repeatCount segment repeat count
 * @param[out] errorOut    error info
 **/
extern void
QBMPDTimelineSetRepeatCount(QBMPDTimeline self,
                            int repeatCount,
                            SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDTIMELINEINTERNAL_H_ */

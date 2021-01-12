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

#ifndef QBMPDTIMELINE_H_
#define QBMPDTIMELINE_H_

/**
 * @file QBMPDTimeline.h
 * @brief Timeline element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBMPDTimeline Timeline element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Timeline element class.
 **/
typedef struct QBMPDTimeline_ *QBMPDTimeline;

/**
 * Get segment start time in timescale units.
 *
 * @param[in] self  Timeline element handle
 * @return          segment start time
 **/
extern uint64_t
QBMPDTimelineGetStartTime(QBMPDTimeline self);

/**
 * Get segment duration in timescale units.
 *
 * @param[in] self  Timeline element handle
 * @return          segment duration
 **/
extern uint64_t
QBMPDTimelineGetDuration(QBMPDTimeline self);

/**
 * Get segment repeat count.
 * Negative value of repeat count indicates, that the value of duration repeats until the start of next segment
 *
 * @param[in]  self     Timeline element handle
 * @return              segment repeat count
 **/
extern int
QBMPDTimelineGetRepeatCount(QBMPDTimeline self);

/**
 * @}
 **/

#endif /* QBMPDTIMELINE_H_ */

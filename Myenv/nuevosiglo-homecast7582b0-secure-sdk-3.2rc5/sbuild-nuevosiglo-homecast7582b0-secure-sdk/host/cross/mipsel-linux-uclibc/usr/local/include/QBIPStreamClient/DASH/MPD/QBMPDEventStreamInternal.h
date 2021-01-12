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

#ifndef QBMPDEVENTSTREAMINTERNAL_H_
#define QBMPDEVENTSTREAMINTERNAL_H_

/**
 * @file QBMPDEventStreamInternal.h
 * @brief Event Stream element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDEvent.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDXlink.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDEventStream.h>

/**
 * @addtogroup QBMPDEventStream
 * @{
 **/

/**
 * Create Event Stream element.
 *
 * @param[in]  schemeIdUri message scheme
 * @param[out] errorOut    error info
 * @return                 new instance of Event Stream element, @c NULL in case of error
 **/
extern QBMPDEventStream
QBMPDEventStreamCreate(SvURI schemeIdUri,
                       SvErrorInfo *errorOut);

/**
 * Set xlink for Event Stream element.
 *
 * @param[in]  self     Event Stream element handle
 * @param[in]  xlink    Xlink element
 * @param[out] errorOut error info
 **/
extern void
QBMPDEventStreamSetXlink(QBMPDEventStream self,
                         QBMPDXlink xlink,
                         SvErrorInfo *errorOut);

/**
 * Set value for Event Stream element.
 *
 * @param[in]  self     Event Stream element handle
 * @param[in]  value    value for the event stream
 * @param[out] errorOut error info
 **/
extern void
QBMPDEventStreamSetValue(QBMPDEventStream self,
                         SvString value,
                         SvErrorInfo *errorOut);

/**
 * Set timescale for Event Stream element.
 *
 * @param[in]  self      Event Stream element handle
 * @param[in]  timescale timescale in units per seconds
 * @param[out] errorOut  error info
 **/
extern void
QBMPDEventStreamSetTimescale(QBMPDEventStream self,
                             uint64_t timescale,
                             SvErrorInfo *errorOut);

/**
 * Add Event element to Event Stream element.
 *
 * @param[in]  self     Event Stream element
 * @param[in]  event    Event element
 * @param[out] errorOut error info
 **/
extern void
QBMPDEventStreamAddEvent(QBMPDEventStream self,
                         QBMPDEvent event,
                         SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDEVENTSTREAMINTERNAL_H_ */

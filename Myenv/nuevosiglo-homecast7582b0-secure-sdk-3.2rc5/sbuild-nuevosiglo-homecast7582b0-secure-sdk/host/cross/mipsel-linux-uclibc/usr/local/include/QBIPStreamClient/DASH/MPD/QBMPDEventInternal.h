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

#ifndef QBMPDEVENTINTERNAL_H_
#define QBMPDEVENTINTERNAL_H_

/**
 * @file QBMPDEventInternal.h
 * @brief Event element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDEvent.h>

/**
 * @addtogroup QBMPDEvent
 * @{
 **/

/**
 * Create Event element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Event element, @c NULL in case of error
 **/
extern QBMPDEvent
QBMPDEventCreate(SvErrorInfo *errorOut);

/**
 * Set presentation time for Event element.
 *
 * @param[in]  self             Event element handle
 * @param[in]  presentationTime presentation time of the event in @c timescale units
 * @param[out] errorOut         error info
 **/
extern void
QBMPDEventSetPresentationTime(QBMPDEvent self,
                              uint64_t presentationTime,
                              SvErrorInfo *errorOut);

/**
 * Set presentation duration for Event element.
 *
 * @param[in]  self     Event element handle
 * @param[in]  duration presentation duration of the event in @c timescale units
 * @param[out] errorOut error info
 **/
extern void
QBMPDEventSetDuration(QBMPDEvent self,
                      uint64_t duration,
                      SvErrorInfo *errorOut);

/**
 * Set identifier for Event element.
 *
 * @param[in]  self     Event element handle
 * @param[in]  id       identifier of the event
 * @param[out] errorOut error info
 **/
extern void
QBMPDEventSetId(QBMPDEvent self,
                unsigned int id,
                SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDEVENTINTERNAL_H_ */

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

#ifndef QBMPDBASEURLINTERNAL_H_
#define QBMPDBASEURLINTERNAL_H_

/**
 * @file QBMPDBaseURLInternal.h
 * @brief Base URL element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <stdbool.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDBaseURL.h>

/**
 * @addtogroup QBMPDBaseURL
 * @{
 **/

/**
 * Create Base URL element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Base URL, @c NULL in case of error
 **/
extern QBMPDBaseURL
QBMPDBaseURLCreate(SvErrorInfo *errorOut);

/**
 * Set URI for Base URL element.
 *
 * @param[in]  self     Base URL element handle
 * @param[in]  uri      URI that can be used as based URL
 * @param[out] errorOut error info
 **/
extern void
QBMPDBaseURLSetURI(QBMPDBaseURL self,
                   SvURI uri,
                   SvErrorInfo *errorOut);

/**
 * Set service location for Base URL element.
 *
 * @param[in]  self            Base URL element handle
 * @param[in]  serviceLocation service location
 * @param[out] errorOut        error info
 **/
extern void
QBMPDBaseURLSetServiceLocation(QBMPDBaseURL self,
                               SvString serviceLocation,
                               SvErrorInfo *errorOut);

/**
 * Set bytes ragne for Base URL element.
 *
 * @param[in]  self       Base URL element handle
 * @param[in]  bytesRange bytes range template
 * @param[out] errorOut   error info
 **/
extern void
QBMPDBaseURLSetByteRange(QBMPDBaseURL self,
                         SvString bytesRange,
                         SvErrorInfo *errorOut);

/**
 * Set availability time offset for Base URL element.
 *
 * @param[in]  self                   Base URL element handle
 * @param[in]  availabilityTimeOffset availability time offset
 * @param[out] errorOut               error info
 **/
extern void
QBMPDBaseURLSetAvailabilityTimeOffset(QBMPDBaseURL self,
                                      double availabilityTimeOffset,
                                      SvErrorInfo *errorOut);

/**
 * Set availability time complete for Base URL element.
 *
 * @param[in]  self                     Base URL element handle
 * @param[in]  availabilityTimeComplete availability time complete
 * @param[out] errorOut                 error info
 **/
extern void
QBMPDBaseURLSetAvailabilityTimeComplete(QBMPDBaseURL self,
                                        bool availabilityTimeComplete,
                                        SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDBASEURLINTERNAL_H_ */

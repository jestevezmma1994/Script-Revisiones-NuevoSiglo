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

#ifndef QBMPDURLINTERNAL_H_
#define QBMPDURLINTERNAL_H_

/**
 * @file QBMPDURLInternal.h
 * @brief URL element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDURL.h>

/**
 * @addtogroup QBMPDURL
 * @{
 **/

/**
 * Create URL element.
 *
 * @param[out] errorOut  error info
 * @return               new instance of URL element, @c NULL in case of error
 **/
extern QBMPDURL
QBMPDURLCreate(SvErrorInfo *errorOut);

/**
 * Set source URL for URL element.
 *
 * @param[in]  self      URL element handle
 * @param[in]  sourceURL source URL (it shall be formatted as absolute URI or relative reference)
 * @param[out] errorOut  error info
 **/
extern void
QBMPDURLSetSourceURL(QBMPDURL self,
                     SvURI sourceURL,
                     SvErrorInfo *errorOut);

/**
 * Set bytes range for URL element.
 *
 * @param[in]  self     URL element handle
 * @param[in]  range    bytes range
 * @param[out] errorOut error info
 **/
extern void
QBMPDURLSetRange(QBMPDURL self,
                 SvString range,
                 SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDURLINTERNAL_H_ */

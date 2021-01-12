/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
****************************************************************************/

#ifndef QB_DVB_SUBS_H
#define QB_DVB_SUBS_H

#include "QBDvbSubsPage.h"
#include "QBDvbSubsBuff.h"

#define QB_DVB_SUBS_WINDOW_H 576
#define QB_DVB_SUBS_WINDOW_W 720

#define QB_DVB_SUBS_WINDOW_H_MAX 1080
#define QB_DVB_SUBS_WINDOW_W_MAX 1920

#ifdef __cplusplus
extern "C" {
#endif

/** Parse PES data into a single subtitle page.
 *  \param data  full pes (starting with 000001 start code).
 *  \param len  length of \a data
 *  \returns parsed page object, or null on error
 */
QBDvbSubsPage  QBDvbSubsParsePes(const unsigned char* data, int len, QBDvbSubsBuff buff);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DVB_SUBS_PAGE_H

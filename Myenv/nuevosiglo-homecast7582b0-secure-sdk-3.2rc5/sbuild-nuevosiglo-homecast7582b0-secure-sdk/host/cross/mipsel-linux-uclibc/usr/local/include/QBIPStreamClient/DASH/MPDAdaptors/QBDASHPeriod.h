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

#ifndef QBIPSTREAMCLIENT_DASH_QBDASHPERIOD_H_
#define QBIPSTREAMCLIENT_DASH_QBDASHPERIOD_H_

/**
 * @file QBDASHPeriod.h MPEG-DASH Period
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBDASHPeriod MPEG-DASH MPD Period element
 * @ingroup QBIPStreamDASHSupport
 * @{
 * High-level representation of MPEG-DASH MPD Period element
 **/

/**
 * QBDASHPeriod.
 * @class QBDASHPeriod
 * @extends SvObject
 */
typedef struct QBDASHPeriod_ *QBDASHPeriod;

/**
 * Get DASH MPD AdaptationSets of given type
 *
 * @memberof QBDASHPeriod
 *
 * @param[in] self QBDASHPeriod object
 * @param[in] type type of MPD AdaptationSets to get
 * @return array of MPD AdaptationSets of given type
 */
SvArray
QBDASHPeriodGetMPDAdaptationSets(QBDASHPeriod self, QBDASHAdaptationSetType type);

/**
 * Get DASH AdaptationSet representing given MPD AdaptationSet
 *
 * @memberof QBDASHPeriod
 *
 * @param[in] self QBDASHPeriod object
 * @param[in] mpdAdaptationSet MPD AdaptationSet that will be represented by DASH AdaptationSet.
 * @return DASH AdaptationSet corresponding to given MPD AdaptationSet
 */
QBDASHAdaptationSet
QBDASHPeriodGetAdaptationSet(QBDASHPeriod self, QBMPDAdaptationSet mpdAdaptationSet);

/**
 * Get runtime type identification object representing QBDASHPeriod class.
 *
 * @relates QBDASHPeriod
 *
 * @return QBDASHPeriod runtime type identification object
 **/
SvType
QBDASHPeriod_getType(void);

/**
 * Create new DASH Period object.
 *
 * @memberof QBDASHPeriod
 *
 * @param[in] mpd MPEG-DASH MPD Element
 * @param[in] mpdPeriod MPEG-DASH MPD Period
 * @param[in] manifestBaseURL DASH manifest base URL
 * @param[out] errorOut error information
 * @return created DASH Period object, or @c NULL in case of error
 */
QBDASHPeriod
QBDASHPeriodCreate(QBMPDElement mpd, QBMPDPeriod mpdPeriod, SvURL manifestBaseURL, SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_DASH_QBDASHPERIOD_H_ */

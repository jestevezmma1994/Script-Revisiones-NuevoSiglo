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

#ifndef QBIPSTREAMCLIENT_DASH_QBDASHPERIODINTERNAL_H_
#define QBIPSTREAMCLIENT_DASH_QBDASHPERIODINTERNAL_H_

/**
 * @file QBDASHPeriodInternal.h MPEG-DASH Period internals
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvObject.h>

/**
 * @defgroup QBDASHPeriodInternal DASH Segment internals
 * @{
 * High-level MPEG-DASH Period internals
 **/

/**
 * QBDASHPeriod.
 * @class QBDASHPeriod
 * @extends SvObject
 */
struct QBDASHPeriod_ {
    /** super object */
    struct SvObject_ super_;

    /** MPEG-DASH MPD Element */
    QBMPDElement mpd;

    /** MPEG-DASH MPD Period */
    QBMPDPeriod mpdPeriod;

    /** DASH manifest base URL */
    SvURL manifestBaseURL;

    /** MPD AdaptationSets of different types*/
    SvArray adaptationSetsByType[QBDASHAdaptationSetType_guard];

    /** MPD AdaptationSet to DASH AdaptationSet mapping */
    SvHashTable adaptationSetsByMPD;
};

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_DASH_QBDASHPERIODINTERNAL_H_ */

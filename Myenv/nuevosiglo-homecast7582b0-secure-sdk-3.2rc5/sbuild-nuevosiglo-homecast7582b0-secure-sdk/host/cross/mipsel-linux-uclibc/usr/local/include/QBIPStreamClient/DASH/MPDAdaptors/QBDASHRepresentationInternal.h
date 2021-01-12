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

#ifndef QBIPSTREAMCLIENT_QBDASHREPRESENTATIONINTERNAL_H_
#define QBIPSTREAMCLIENT_QBDASHREPRESENTATIONINTERNAL_H_

/**
 * @file QBDASHRepresentationInternal.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHRepresentation.h>
#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHSegment.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentation.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBDASHRepresentationInternal DASH Representation internals
 * @ingroup QBIPStreamDASHSupport
 * @{
 * High-level MPEG-DASH Representation internals
 **/

/**
 * QBDASHRepresentation.
 * @class QBDASHRepresentation
 * @extends SvObject
 */
struct QBDASHRepresentation_ {

    /** super class */
    struct SvObject_ super_;

    /** MPEG-DASH MPD Element */
    QBMPDElement mpd;

    /** MPEG-DASH MPD Period */
    QBMPDPeriod mpdPeriod;

    /** MPEG-DASH MPD Adaptation Set */
    QBMPDAdaptationSet mpdAdaptationSet;

    /** MPEG-DASH MPD Representation */
    QBMPDRepresentation mpdRepresentation;

    /** MPEG-DASH manifest base URL */
    SvURL manifestBaseURL;

    /** Initialization segment */
    QBDASHSegment initializationSegment;

    /** Bitstream Switching segment*/
    QBDASHSegment bitstreamSwitchingSegment;

    /** Media Segment */
    QBDASHSegment mediaSegment;

    /** Index Segment */
    QBDASHSegment indexSegment;

};

/**
 * Get base part of URL for current Representation.
 *
 * @param[in] self DASH Representation object
 * @return base part of URL
 */
SvURL
QBDASHRepresentationCreateBaseURL(QBDASHRepresentation self);

/**
 * Create byte range for current Representation
 * @param[in] self DASH Representation object
 * @return byte range
 */
SvString
QBDASHRepresentationCreateByteRange(QBDASHRepresentation self);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBDASHREPRESENTATIONINTERNAL_H_ */

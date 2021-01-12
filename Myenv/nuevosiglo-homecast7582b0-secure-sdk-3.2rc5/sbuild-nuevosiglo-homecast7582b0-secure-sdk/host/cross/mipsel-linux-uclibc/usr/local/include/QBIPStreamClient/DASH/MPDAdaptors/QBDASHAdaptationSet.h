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

#ifndef QBIPSTREAMCLIENT_QBDASHADAPTATIONSET_H_
#define QBIPSTREAMCLIENT_QBDASHADAPTATIONSET_H_

/**
 * @file QBDASHAdaptationSet.h MPEG-DASH Adaptation Set
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHSegment.h>
#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHRepresentation.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentation.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBDASHAdaptationSet MPEG-DASH MPD Adaptation Set
 * @ingroup QBIPStreamDASHSupport
 * @{
 * High-level representation of MPEG-DASH MPD Adaptation Set element
 *
 **/

/**
 * QBDASHAdaptationSet.
 * @class QBDASHAdaptationSet
 * @extends SvObject
 */
typedef struct QBDASHAdaptationSet_ *QBDASHAdaptationSet;

/**
 * DASH Adaptation Set type.
 * An Adaptation Set contains alternate Representations.
 * Section 5.3.3 ISO/IEC 23009-1:2014
 */
typedef enum {
    QBDASHAdaptationSetType_unknown,     //!< unknown type
    QBDASHAdaptationSetType_audio,       //!< adaptation set of audio type
    QBDASHAdaptationSetType_video,       //!< adaptation set of video type
    QBDASHAdaptationSetType_text,        //!< adaptation set of text type
    QBDASHAdaptationSetType_application, //!< adaptation set of application type
    QBDASHAdaptationSetType_guard,       //!< enum guard
} QBDASHAdaptationSetType;

/**
 * Create QBDASHAdaptationSet object.
 *
 * @memberof QBDASHAdaptationSet
 *
 * @param[in] type              type of DASH adaptation set
 * @param[in] mpdElement        DASH parser MPD representation
 * @param[in] mpdPeriod         DASH parser MPD Period element representation
 * @param[in] mpdAdaptationSet  DASH parser MPD Adaptation Set element representation
 * @param[in] manifestBaseURL   DASH manifest base URL
 * @param[out] errorOut         error information
 * @return created DASH AdaptationSet object, or @c NULL in case of error
 */
QBDASHAdaptationSet
QBDASHAdaptationSetCreate(QBDASHAdaptationSetType type,
                          QBMPDElement mpdElement,
                          QBMPDPeriod mpdPeriod,
                          QBMPDAdaptationSet mpdAdaptationSet,
                          SvURL manifestBaseURL,
                          SvErrorInfo *errorOut);

/**
 * Get DASH representation object corresponding to DASH parser MPD representation.
 * Its ID is used to find corresponding DASH representation object.
 *
 * @memberof QBDASHAdaptationSet
 *
 * @param[in] self DASH Adaptation Set object
 * @param[in] mpdRepresentation     DASH parser MPD representation
 * @param[out] errorOut              error information
 * @return DASH representation object
 */
QBDASHRepresentation
QBDASHAdaptationSetGetRepresentation(QBDASHAdaptationSet self,
                                     QBMPDRepresentation mpdRepresentation,
                                     SvErrorInfo *errorOut);

/**
 * Get type of DASH Adaptation Set object.
 *
 * @memberof QBDASHAdaptationSet
 *
 * @param[in] self DASH Adaptation Set object
 * @return type DASH Adaptation Set object
 */
QBDASHAdaptationSetType
QBDASHAdaptationSetGetType(QBDASHAdaptationSet self);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBDASHADAPTATIONSET_H_ */

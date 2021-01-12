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

#ifndef QBIPSTREAMCLIENT_DASH_QBDASHSEGMENTTEMPLATE_H_
#define QBIPSTREAMCLIENT_DASH_QBDASHSEGMENTTEMPLATE_H_

/**
 * @file QBDASHSegmentTemplate.h MPG-DASH SegmentTemplate representation
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentation.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBDASHSegmentTemplate MPG-DASH MPD SegmentTemplate element
 * @{
 * High-level representation of MPEG-DASH MPD SegmentTemplate element
 **/

/**
 * QBDASHSegmentTemplate.
 * @class QBDASHSegmentTemplate
 * @extends QBDASHRepresentation
 */
typedef struct QBDASHSegmentTemplate_ *QBDASHSegmentTemplate;

/**
 * Get runtime type identification object representing QBDASHSegmentTemplate class.
 *
 * @relates QBDASHSegmentTemplate
 *
 * @return QBDASHSegmentTemplate runtime type identification object
 **/
SvType
QBDASHSegmentTemplate_getType(void);

/**
 * Create QBDASHSegment object.
 *
 * @param[in] mpd DASH parser MPD representation
 * @param[in] mpdPeriod DASH parser MPD Period element representation
 * @param[in] mpdAdaptationSet DASH parser MPD Adaptation Set element representation
 * @param[in] mpdRepresentation DASH parser MPD Representation element representation
 * @param[in] manifestBaseURL   DASH manifest base URL
 * @param[out] errorOut error information
 * @return created DASH SegmentTemplate object, or @c NULL in case of error
 */
QBDASHSegmentTemplate
QBDASHSegmentTemplateCreate(QBMPDElement mpd, QBMPDPeriod mpdPeriod,
                            QBMPDAdaptationSet mpdAdaptationSet,
                            QBMPDRepresentation mpdRepresentation,
                            SvURL manifestBaseURL,
                            SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_DASH_QBDASHSEGMENTTEMPLATE_H_ */

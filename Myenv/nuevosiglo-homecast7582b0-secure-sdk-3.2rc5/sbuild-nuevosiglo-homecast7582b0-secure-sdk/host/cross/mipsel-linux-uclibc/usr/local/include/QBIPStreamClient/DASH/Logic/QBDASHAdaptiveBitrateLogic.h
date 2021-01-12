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

#ifndef QB_DASH_ADAPTIVE_BITRATE_LOGIC_H_
#define QB_DASH_ADAPTIVE_BITRATE_LOGIC_H_

/**
 * @file  QBDASHAdaptiveBitrateLogic.h MPEG-DASH adaptive bitrate logic.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/Common/QBIPStreamBitrateManager.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentation.h>
#include <SvPlayerKit/SvPlayerConfig.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBDASHAdaptiveBitrateLogic MPEG-DASH adaptive bitrate logic
 * @{
 **/

/**
 * MPEG-DASH adaptive bitrate logic. Basing on current bitrate,
 * selects which representation should choose.
 * @class QBDASHAdaptiveBitrateLogic
 * @extends SvObject
 */
typedef struct QBDASHAdaptiveBitrateLogic_ *QBDASHAdaptiveBitrateLogic;

/**
 * Create MPEG-DASH adaptive bitrate logic.
 *
 * @param[in] playerConfig      player config, used for choosing the bitrate selector
 * @param[out] errorOut         error info
 * @return                      created logic, @c NULL in case of error
 */
QBDASHAdaptiveBitrateLogic
QBDASHAdaptiveBitrateLogicCreate(SvPlayerConfig playerConfig,
                                 SvErrorInfo *errorOut);

/**
 * Apply new MPD adaptation set for which logic should compute MPD representation based on current network conditions.
 *
 * @param[in]  self             MPEG-DASH adaptive bitrate logic
 * @param[in]  adaptationSet    new adaptation set
 * @param[out] errorOut         error info
 */
void
QBDASHAdaptiveBitrateLogicApplyAdaptationSet(QBDASHAdaptiveBitrateLogic self,
                                             QBMPDAdaptationSet adaptationSet,
                                             SvErrorInfo *errorOut);

/**
 * Get MPD representation based on current network conditions.
 *
 * For compute current MPD representation, MPD adaptation set must be applied (see @ref QBDASHAdaptiveBitrateLogicApplyAdaptationSet)
 *
 * @param[in] self              MPEG-DASH adaptive bitrate logic
 * @param[in] bitrateManager    bitrate manager, provides current real bitrate
 * @return                      MPD representation, otherwise @c NULL in case of error
 */
QBMPDRepresentation
QBDASHAdaptiveBitrateLogicGetRepresentation(QBDASHAdaptiveBitrateLogic self, QBIPStreamBitrateManager bitrateManager);

/**
 * Get MPD representation.
 *
 * This method returns currently selected representation.
 * In case there is no representation selected then @ref QBDASHAdaptiveBitrateLogicApplyAdaptationSet is called internally.
 *
 * @param[in] self              MPEG-DASH adaptive bitrate logic
 * @param[in] bitrateManager    bitrate manager, provides current real bitrate
 * @return                      MPD representation, otherwise @c NULL in case of error
 */
QBMPDRepresentation
QBDASHAdaptiveBitrateLogicGetLastRepresentation(QBDASHAdaptiveBitrateLogic self, QBIPStreamBitrateManager bitrateManager);

/**
 * @}
 */

#endif /* QB_DASH_ADAPTIVE_BITRATE_LOGIC_H_ */

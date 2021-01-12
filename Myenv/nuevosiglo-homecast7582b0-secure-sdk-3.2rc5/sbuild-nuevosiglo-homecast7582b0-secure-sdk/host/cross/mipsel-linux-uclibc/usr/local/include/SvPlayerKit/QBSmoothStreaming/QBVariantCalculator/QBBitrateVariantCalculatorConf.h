/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_BITRATE_VARIANT_CALCULATOR_CONF_H_
#define QB_BITRATE_VARIANT_CALCULATOR_CONF_H_

/**
 * @file QBBitrateVariantCalculatorConf.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>

#include <stdbool.h>
/**
 * @defgroup QBBitrateVariantCalculatorConf QBBitrateVariantCalculatorConf class
 * @ingroup CubiTV
 * @{
 **/

/**
 * @brief Indicates from which bitrate playback should start.
 */
typedef enum {
    QBBitrateVariantCalculatorConfInitialBitrateLevel_highest,    ///< Start playback from highest bitrate
    QBBitrateVariantCalculatorConfInitialBitrateLevel_middle,     ///< Start playback from middle bitrate
    QBBitrateVariantCalculatorConfInitialBitrateLevel_lowest      ///< Start playback from lowest bitrate
} QBBitrateVariantCalculatorConfInitialBitrateLevel;

/**
 * QBBitrateVariantCalculatorConf class.
 *
 * @class QBBitrateVariantCalculatorConf
 * @extends SvObject
 **/
typedef struct QBBitrateVariantCalculatorConf_ *QBBitrateVariantCalculatorConf;

/**
 * Get runtime type identification object representing the QBBitrateVariantCalculatorConf class.
 *
 * @relates QBBitrateVariantCalculatorConf
 *
 * @return QBBitrateVariantCalculatorConf type identification object
 **/
SvType
QBBitrateVariantCalculatorConf_getType(void);

/**
 * Create QBBitrateVariantCalculatorConf.
 *
 * @param[out] errorOut error info
 *
 * @return created QBBitrateVariantCalculatorConf, @c NULL in case of error
 **/
QBBitrateVariantCalculatorConf
QBBitrateVariantCalculatorConfCreate(SvErrorInfo *errorOut);

/**
 * Create QBBitrateVariantCalculatorConf.
 *
 * @param[in]  settings dictinary with bitrate configuration
 * @param[out] errorOut error info
 *
 * @return created QBBitrateVariantCalculatorConf, @c NULL in case of error
 **/
QBBitrateVariantCalculatorConf
QBBitrateVariantCalculatorConfCreateFromSettings(SvHashTable settings, SvErrorInfo *errorOut);

/**
 * Get threshold for which algorithm, calculating next quality level, should jump up.
 *
 * @param[in] self   instance of QBBitrateVariantCalculatorConf
 * @param[in] isLive type of player
 *
 * @return threshold in percents
 */
int
QBBitrateVariantCalculatorConfGetThresholdJumpUp(QBBitrateVariantCalculatorConf self, bool isLive);

/**
 * Get threshold for which algorithm, calculating next quality level, should jump down.
 *
 * @param[in] self   instance of QBBitrateVariantCalculatorConf
 * @param[in] isLive type of player
 *
 * @return threshold in percents
 */
int
QBBitrateVariantCalculatorConfGetThresholdJumpDown(QBBitrateVariantCalculatorConf self, bool isLive);

/**
 * Get level of initial bitrate for playback.
 *
 * @param[in] self   instance of QBBitrateVariantCalculatorConf
 *
 * @return initial bitrate
 **/
QBBitrateVariantCalculatorConfInitialBitrateLevel
QBBitrateVariantCalculatorConfGetInitialBitrateLevel(QBBitrateVariantCalculatorConf self);


/**
 * @}
 **/

#endif // QB_BITRATE_VARIANT_CALCULATOR_CONF_H_

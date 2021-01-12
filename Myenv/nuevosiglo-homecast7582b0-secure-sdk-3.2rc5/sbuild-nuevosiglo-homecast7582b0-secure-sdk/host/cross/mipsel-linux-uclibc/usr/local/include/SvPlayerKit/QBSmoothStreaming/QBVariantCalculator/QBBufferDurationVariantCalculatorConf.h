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

#ifndef QB_BUFFER_DURATION_VARIANT_CALCULATOR_CONF_H_
#define QB_BUFFER_DURATION_VARIANT_CALCULATOR_CONF_H_

/**
 * @file QBBufferDurationVariantCalculatorConf.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvHashTable.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBBufferDurationVariantCalculatorConf QBBufferDurationVariantCalculatorConf class
 * @ingroup CubiTV
 * @{
 **/

/**
 * @brief Indicates from which bitrate playback should start.
 */
typedef enum {
    QBBufferDurationVariantCalculatorConfInitialBitrateLevel_highest,    ///< Start playback from highest bitrate
    QBBufferDurationVariantCalculatorConfInitialBitrateLevel_middle,     ///< Start playback from middle bitrate
    QBBufferDurationVariantCalculatorConfInitialBitrateLevel_lowest      ///< Start playback from lowest bitrate
} QBBufferDurationVariantCalculatorConfInitialBitrateLevel;

/**
 * QBBufferDurationVariantCalculatorConf class.
 *
 * @class QBBufferDurationVariantCalculatorConf
 * @extends SvObject
 **/
typedef struct QBBufferDurationVariantCalculatorConf_ *QBBufferDurationVariantCalculatorConf;

/**
 * Get runtime type identification object representing the QBBufferDurationVariantCalculatorConf class.
 *
 * @relates QBBufferDurationVariantCalculatorConf
 *
 * @return QBBufferDurationVariantCalculatorConf type identification object
 **/
SvType
QBBufferDurationVariantCalculatorConf_getType(void);

/**
 * Create QBBufferDurationVariantCalculatorConf.
 *
 * @param[out] errorOut error info
 *
 * @return created QBBufferDurationVariantCalculatorConf, @c NULL in case of error
 **/
QBBufferDurationVariantCalculatorConf
QBBufferDurationVariantCalculatorConfCreate(SvErrorInfo *errorOut);

/**
 * Create QBBufferDurationVariantCalculatorConf.
 *
 * @param[in]  settings dictinary with bufferDuration configuration
 * @param[out] errorOut error info
 *
 * @return created QBBufferDurationVariantCalculatorConf, @c NULL in case of error
 **/
QBBufferDurationVariantCalculatorConf
QBBufferDurationVariantCalculatorConfCreateFromSettings(SvHashTable settings, SvErrorInfo *errorOut);

/**
 * Get jumps function
 *
 * It is defined as matrix.
 * First index is speed and seccond is buffer duration.
 * Speed and duration are rounded to floor.
 * They are limited by buffer ceiling and speed ceiling accordingly.
 *
 * @param[in] self instance of QBBufferDurationVariantCalculatorConf
 *
 * @return variant jumps matrix
 **/
SvArray
QBBufferDurationVariantCalculatorConfGetJumps(QBBufferDurationVariantCalculatorConf self);

/**
 * Get upper limit of buffer duration domain of jumps function
 * Buffer duration above this limit is mapped to this limit
 *
 * @param[in] self instance of QBBufferDurationVariantCalculatorConf
 *
 * @return buffer upper limit of jumps function
 */
double
QBBufferDurationVariantCalculatorConfGetJumpBufferCeiling(QBBufferDurationVariantCalculatorConf self);

/**
 * Get upper limit of speed domain of jumps function
 * Speed above this limit is mapped to this limit
 *
 * @param[in] self instance of QBBufferDurationVariantCalculatorConf
 *
 * @return speed upper limit of jumps function
 */
double
QBBufferDurationVariantCalculatorConfGetJumpSpeedCeiling(QBBufferDurationVariantCalculatorConf self);

/**
 * Get maximum buffer duration (in terms of chunk duration) loosing above which we will ignore jump up
 *
 * @param[in] self instance of QBBufferDurationVariantCalculatorConf
 *
 * @return limit for jumping up (one mean number of seconds equal to chunk duration)
 */
double
QBBufferDurationVariantCalculatorConfGetLoosingChunkSizeGuard(QBBufferDurationVariantCalculatorConf self);

/**
 * Get maximum buffer duration (in seconds) losing above which we will ignore jump up
 *
 * @param[in] self instance of QBBufferDurationVariantCalculatorConf
 *
 * @return limit for jumping up (in seconds)
 */
double
QBBufferDurationVariantCalculatorConfGetLoosingDurationGuard(QBBufferDurationVariantCalculatorConf self);

/**
 * Get buffer duration below which we will force jump down while loosing buffer
 *
 * @param[in] self instance of QBBufferDurationVariantCalculatorConf
 *
 * @return limit in seconds
 */
double
QBBufferDurationVariantCalculatorConfGetSafeToLooseDuration(QBBufferDurationVariantCalculatorConf self);

/**
 * Get buffer duration above which we will ignore jump down while buffer is growing
 *
 * @param[in] self instance of QBBufferDurationVariantCalculatorConf
 *
 * @return limit in seconds
 **/
double
QBBufferDurationVariantCalculatorConfGetCriticalDuration(QBBufferDurationVariantCalculatorConf self);

/**
 * Get level of initial bitrate for playback.
 *
 * @param[in] self instance of QBBufferDurationVariantCalculatorConf
 *
 * @return initial bitrate
 **/
QBBufferDurationVariantCalculatorConfInitialBitrateLevel
QBBufferDurationVariantCalculatorConfGetInitialBitrateLevel(QBBufferDurationVariantCalculatorConf self);

/**
 * @}
 **/
#endif // QB_BUFFER_DURATION_VARIANT_CALCULATOR_CONF_H_

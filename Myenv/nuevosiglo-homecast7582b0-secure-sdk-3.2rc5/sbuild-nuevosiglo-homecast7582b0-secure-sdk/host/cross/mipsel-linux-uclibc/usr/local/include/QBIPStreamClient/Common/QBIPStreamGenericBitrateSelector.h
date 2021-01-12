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

#ifndef QB_IP_STREAM_GENERIC_BITRATE_SELECTOR_H_
#define QB_IP_STREAM_GENERIC_BITRATE_SELECTOR_H_

/**
 * @file  QBIPStreamGenericBitrateSelector.h Generic bitrate selector.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBIPStreamBitrateSelectorIface Bitrate selector interface.
 * @{
 **/

/**
 * IP stream generic bitrate selector.
 * @class QBIPStreamGenericBitrateSelector
 * @implements QBIPStreamBitrateSelectorIface
 */
typedef struct QBIPStreamGenericBitrateSelector_ *QBIPStreamGenericBitrateSelector;

/**
 * Create IP stream generic bitrate selector.
 *
 * @param[in] thresholdJumpUp       ratio of current bitrate to representation
 *                                  bitrate, when bitrate selector should jump up
 *                                  with representation bitrate
 * @param[in] thresholdJumpDown     ratio of current bitrate to representation
 *                                  bitrate, when bitrate selector should jump down
 *                                  with representation bitrate
 * @param[out] errorOut             error info
 * @return                          created selector, @c NULL in case of error
 */
SvObject
QBIPStreamGenericBitrateSelectorCreate(double thresholdJumpUp,
                                       double thresholdJumpDown,
                                       SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_IP_STREAM_GENERIC_BITRATE_SELECTOR_H_ */

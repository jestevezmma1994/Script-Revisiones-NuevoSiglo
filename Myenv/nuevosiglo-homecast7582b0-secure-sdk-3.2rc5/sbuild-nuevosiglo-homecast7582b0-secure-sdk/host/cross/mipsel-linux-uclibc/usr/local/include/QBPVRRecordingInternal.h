/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PVR_RECORDING_INTERNAL_H_
#define QB_PVR_RECORDING_INTERNAL_H_

/**
 * @file QBPVRRecordingInternal.h QBPVRRecording class internal methods.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBPVRRecording.h"

#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>
#include <time.h>

/**
 * @addtogroup QBPVRRecording
 * @{
 **/

/**
 * Init PVR recording.
 *
 * @memberof QBPVRRecording
 *
 * @param[in] self              QBPVRRecording handle
 * @param[out] errorOut         error info
 * @return @c <0 in case of error, @c 0 otherwise
 **/
int
QBPVRRecordingInit(QBPVRRecording self,
                   SvErrorInfo *errorOut);

/**
 * Copy PVR recording.
 *
 * @memberof QBPVRRecording
 *
 * @param[in] dest              destination handle
 * @param[in] src               source handle
 **/
void
QBPVRRecordingCopy(QBPVRRecording dest,
                   QBPVRRecording src);

/**
 * @}
 **/

#endif /* QB_PVR_RECORDING_INTERNAL_H_ */

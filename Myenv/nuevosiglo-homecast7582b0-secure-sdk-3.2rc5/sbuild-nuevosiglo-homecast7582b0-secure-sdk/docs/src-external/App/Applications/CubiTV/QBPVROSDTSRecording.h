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


#ifndef QBPVROSDTSRECORDING_H_
#define QBPVROSDTSRECORDING_H_

/**
 * @file QBPVROSDTSRecording.h On screen display base class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Logic/EventsLogic.h>
#include <Services/langPreferences.h>
#include <Services/core/QBTextRenderer.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerManager/SvPlayerTaskState.h>
#include <QBOSD/QBOSD.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>

#include <time.h>


/**
 * @defgroup QBPVROSDTSRecording On Screen Display handler for timeshift recordings.
 * @ingroup QBPVROSDRecording
 * @{
 **/

/**
 * On Screen Display handler for timeshift recordings.
 *
 * @class QBPVROSDTSRecording QBPVROSDTSRecording.h <Widgets/OSD/QBPVROSDRecording.h>
 * @extends QBPVROSDRecording
 **/
typedef struct QBPVROSDTSRecording_ *QBPVROSDTSRecording;

/**
 * @relates QBPVROSDTSRecording
 *
 * Get runtime type identification object representing OSD timeshift recording class.
 *
 * @return QBOSDTSRecording type identification object
 **/
SvType
QBPVROSDTSRecording_getType(void);

/**
 * @param[in]  app             CUIT application handle
 * @param[out] errorOut        errorInfo
 * @param[in]  scheduler       scheduler handle
 * @return                     new instance of QBOSDTSRecording
 *                             or @c NULL in case of error
 **/
QBPVROSDTSRecording
QBPVROSDTSRecordingCreate(SvApplication app,
                          SvScheduler scheduler,
                          SvErrorInfo *errorOut);
/**
 * @}
 **/

#endif /* QBPVROSDTSRECORDING_H_ */

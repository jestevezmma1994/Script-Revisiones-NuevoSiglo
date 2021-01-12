/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_PVR_TASK_H_
#define SV_PVR_TASK_H_

/**
 * @file SvPVRTask.h PVR task class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvType.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvPlayerManager/SvPVRTaskState.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvPVRTask PVR task class
 * @ingroup SvPlayerManagerLibrary
 * @{
 *
 * A class representing single PVR instance.
 **/

/**
 * PVR task class.
 * @class SvPVRTask SvPVRTask.h <SvPlayerManager/SvPVRTask.h>
 **/
typedef struct SvPVRTask_ *SvPVRTask;

/**
 * Get runtime type identification object representing PVR task class.
 *
 * @return SvPVRTask type identification object
 **/
extern SvType
SvPVRTask_getType(void);

/**
 * Close PVR recording.
 *
 * @param[in] self PVR task handle
 **/
extern void
SvPVRTaskClose(SvPVRTask self);

/**
 * Get current state of PVR recording.
 *
 * @param[in] self PVR task handle
 * @return current state of PVR recording
 **/
extern SvPVRTaskState
SvPVRTaskGetState(SvPVRTask self);

/**
 * Pass parameter to/from the underlying media player subsystem.
 *
 * @param[in] self  PVR task handle
 * @param[in] name  name of the option
 * @param[in,out] p value of the option
 * @return          @c 0 in success, @c SV_ERR_INVALID when ignored,
 *                  any other value is a fatal error
 *
 * @internal
 **/
extern int
SvPVRTaskSetOpt(SvPVRTask self,
                const char *name,
                void *p);

/**
 * Get PVR task metadata.
 *
 * @param[in] self PVR task
 * @return metadata, @c NULL in case if there is no metadata for that session
 */
extern SvContentMetaData
SvPVRTaskGetMetaData(SvPVRTask self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

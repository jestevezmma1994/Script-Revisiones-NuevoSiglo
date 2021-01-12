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

#ifndef SV_PVR_TASK_LISTENER_H_
#define SV_PVR_TASK_LISTENER_H_

/**
 * @file SvPVRTaskListener.h PVR task listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h> // needed for SvInterface

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvPVRTaskListener PVR task listener interface
 * @ingroup SvPlayerManagerLibrary
 * @{
 *
 * An interface for classes that receive notifications from PVR tasks.
 **/

/**
 * SvPVRTaskListener interface.
 **/
typedef const struct SvPVRTaskListener_ {
    /**
     * Notifies that recorder state has changed.
     *
     * @note Current state can be acquired using SvPVRTaskGetState().
     *
     * @param[in] self PVR task listener handle
     **/
    void (*stateChanged)(SvObject self);

    /**
     * Notifies about fatal error during recording.
     *
     * @param[in] self PVR task listener handle
     * @param[out] errorOut error info
     **/
    void (*fatalErrorOccured)(SvObject self, SvErrorInfo errorOut);

    /**
     * Notifies that recording has finished.
     *
     * @param[in] self PVR task listener handle
     **/
    void (*recordingFinished)(SvObject self);
} *SvPVRTaskListener;

/**
 * Get runtime type identification object representing PVR task listener interface.
 *
 * @return SvPVRTaskListener interface object
 **/
extern SvInterface
SvPVRTaskListener_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

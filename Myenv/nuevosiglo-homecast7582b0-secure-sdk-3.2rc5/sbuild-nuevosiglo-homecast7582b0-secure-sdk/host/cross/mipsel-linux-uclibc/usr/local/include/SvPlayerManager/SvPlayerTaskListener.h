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

#ifndef SV_PLAYER_TASK_LISTENER_H_
#define SV_PLAYER_TASK_LISTENER_H_

/**
 * @file SvPlayerTaskListener.h Player task listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h> // needed for SvInterface
#include <SvFoundation/SvString.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvPlayerTaskListener Player task listener interface
 * @ingroup SvPlayerManagerLibrary
 * @{
 *
 * An interface for classes that receive notifications from player tasks.
 **/

/**
 * SvPlayerTaskListener interface.
 **/
typedef const struct SvPlayerTaskListener_ {
    /**
     * Notifies that player state has changed.
     *
     * @note Current state can be acquired using SvPlayerTaskGetState().
     *
     * @param[in] self player task listener handle
     **/
    void (*stateChanged)(SvObject self);

    /**
     * Notifies about fatal error during playback.
     *
     * @note When this method is called, the task is already inactive,
     * like after calling SvPlayerTaskClose().
     *
     * @param[in] self player task listener handle
     * @param[out] errorOut error info
     **/
    void (*fatalErrorOccured)(SvObject self, SvErrorInfo errorOut);

    /**
     * Notifies about non-fatal error during playback.
     *
     * @param[in] self player task listener handle
     * @param[out] errorOut error info
     **/
    void (*nonFatalErrorOccured)(SvObject self, SvErrorInfo errorOut);

    /**
     * Notifies that player finished outputting content.
     *
     * @param[in] self player task listener handle
     **/
    void (*playbackFinished)(SvObject self);

    /**
     * Notifies about some miscelleaneous events (for example: signal lost/restored,
     * input EOS, discovered subtitles in the stream).
     *
     * @param[in] self player task listener handle
     * @param[in] name name of the event
     * @param[in] arg event-specific information
     **/
    void (*handleEvent)(SvObject self, SvString name, void *arg);
} *SvPlayerTaskListener;

/**
 * Get runtime type identification object representing player task listener interface.
 *
 * @return SvPlayerTaskListener interface object
 **/
extern SvInterface
SvPlayerTaskListener_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

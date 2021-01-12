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

#ifndef QBIPSTREAMMANAGERLISTENER_H
#define QBIPSTREAMMANAGERLISTENER_H

/**
 * @file QBIPStreamManagerListener.h
 * @brief QBIPStreamManager's listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/Common/QBIPStreamManifestPlaybackInfo.h>
#include <QBIPStreamClient/Common/QBIPStreamManagerTypes.h>

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBIPStreamManagerListener QBIPStreamManager's listener interface
 * @{
 **/

/**
 * QBIPStreamManagerListener interface.
 **/
typedef const struct QBIPStreamManagerListener_ {
    /**
     * Handle playback information from MPD manifest.
     *
     * @param[in] self_         listener object handle
     * @param[in] playbackInfo  object containing playback information
     **/
    void (*manifestPlaybackInfo)(SvObject self_,
                                 QBIPStreamManifestPlaybackInfo playbackInfo);

    /**
     * Handle player event.
     *
     * @param[in] self listener object handle
     * @param[in] name event name
     * @param[in] arg  event argument
     **/
    void (*playerEvent)(SvObject self_,
                        SvString name,
                        void *arg);

    /**
     * Handle QBIPStreamManager's error.
     *
     * @param[in] self_         listener object handle
     * @param[in] streamType    stream type on which the error has occured
     * @param[in] errorType     error type
     **/
    void (*error)(SvObject self_,
                  QBIPStreamManagerStreamType streamType,
                  QBIPStreamManagerErrorType errorType);
} *QBIPStreamManagerListener;

/**
 * Get runtime identification object representing
 * QBIPStreamManagerListener interface.
 *
 * @return QBIPStreamManagerListener interface
 **/
extern SvInterface
QBIPStreamManagerListener_getInterface(void);

/**
 * @}
 **/

#endif // QBIPSTREAMMANAGERLISTENER_H

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

#ifndef QBIPSTREAMCLIENT_QBHLSMANAGERLISTENER_H_
#define QBIPSTREAMCLIENT_QBHLSMANAGERLISTENER_H_

/**
 * @file QBHLSManagerListener.h HLS manager listener
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvInterface.h>
#include <QBIPStreamClient/HLS/QBHLS/QBHLSManager.h>

/**
 * @defgroup QBHLSManagerListener HLS manager listener
 * @ingroup QBHLSManager
 * @{
 * HLS manager listener
 **/

/**
 * QBHLSManagerListener interface.
 **/
typedef struct QBHLSManagerListener_ {

    /**
     * Handle new data.
     *
     * @param[in] self_         listener object handle
     * @param[in] sb            data
     **/
    void (*data)(SvObject self_,
                 SvBuf sb);

    /**
     * Handle change of playback's state.
     *
     * @param[in] self_         listener object handle
     * @param[in] playbackState current state of playback
     **/
    void (*playbackStateChanged)(SvObject self_,
                                 QBHLSManagerPlaybackState playbackState);

    /**
     * Handle QBHLSManager's error.
     *
     * @param[in] self_         listener object handle
     * @param[in] errorType     error type
     **/
    void (*error)(SvObject self_,
                  QBHLSManagerErrorType errorType);

} *QBHLSManagerListener;

/**
 * Get runtime identification object representing
 * QBHLSManagerListener interface.
 *
 * @return QBHLSManagerListener interface
 **/
SvInterface
QBHLSManagerListener_getInterface(void);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBHLSMANAGER_H_ */

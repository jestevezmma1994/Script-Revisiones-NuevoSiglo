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

#ifndef QBIPSTREAMFRAMESLISTENER_H
#define QBIPSTREAMFRAMESLISTENER_H

/**
 * @file QBIPStreamFramesListener.h
 * @brief QBIPStreamFrames's listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/Common/QBIPStreamManagerTypes.h>

#include <SvPlayerKit/SvChbuf.h>

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvCoreTypes.h>

#include <stdint.h>

/**
 * @defgroup QBIPStreamFramesListener QBIPStreamFrames's listener interface
 * @{
 **/

/**
 * QBIPStreamFramesListener interface.
 **/
typedef const struct QBIPStreamFramesListener_ {

    /**
     * Handle discontinuity.
     *
     * @param[in] self       listener object handle
     * @param[in] pts90k     position in 90 kHz units
     * @param[in] acceptable @c true if discontinuity is acceptable by sinks
     **/
    void (*discontinuity)(SvObject self_,
                          long long int pts90k,
                          bool acceptable);

    /**
     * Handle end of stream.
     *
     * @param[in] self listener object handle
     **/
    void (*endOfStream)(SvObject self_);

    /**
     * Handle incoming frame.
     *
     * @param[in] self_         listener object handle
     * @param[in] chb           buffer containing frame
     * @param[in] streamType    stream type of the frame
     * @param[in] streamNumber  stream identifier as specified in svdataformat
     * @param[in] pts           frame's presentation timestamp
     * @param[in] duration      duration of the frame
     **/
    void (*frame)(SvObject self_,
                  SvChbuf chb,
                  QBIPStreamManagerStreamType streamType,
                  short streamNumber,
                  uint64_t pts,
                  uint64_t duration);

    /**
     * Handle change of playback's state.
     *
     * @param[in] self_         listener object handle
     * @param[in] streamType    stream type on which the playback has changed its state
     * @param[in] playbackState current state of playback
     **/
    void (*playbackStateChanged)(SvObject self_,
                                 QBIPStreamManagerStreamType streamType,
                                 QBIPStreamManagerPlaybackState playbackState);

} *QBIPStreamFramesListener;

/**
 * Get runtime identification object representing
 * QBIPStreamFramesListener interface.
 *
 * @return QBIPStreamFramesListener interface
 **/
extern SvInterface
QBIPStreamFramesListener_getInterface(void);

/**
 * @}
 **/

#endif // QBIPSTREAMFRAMESLISTENER_H

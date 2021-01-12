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

#ifndef QBDASHFRAMESOURCELISTENER_H
#define QBDASHFRAMESOURCELISTENER_H

/**
 * @file QBDASHFrameSourceListener.h
 * @brief QBDASHFrameSource's listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/Manager/QBDASHFrameSource.h>
#include <QBIPStreamClient/MP4/QBMP4Parser/QBMP4Data.h>

#include <SvPlayerKit/SvChbuf.h>

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvCoreTypes.h>

#include <SvCore/SvErrorInfo.h>

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup QBDASHFrameSourceListener QBDASHFrameSource's listener interface
 * @ingroup QBDASHFrameSource
 * @{
 **/

struct QBDASHMP4FrameData {
    SvChbuf chb;            ///< parsed frame
    uint32_t trackIndex;    ///< index of track for which frame belongs (only in file mode)
    uint64_t timescale;     ///< timescale value
    uint64_t pts;           ///< presentation time stamp of frame (in file mode in 90 kHz units, in fragmented mode in timescale units)
    int64_t segmentPTS;     ///< presentation time stamp of segment (available only in fragmented mode in timescale units), -1 if not available
    uint64_t duration;      ///< duration of frame (only in fragmented mode)
};

/**
 * QBDASHFrameSourceListener interface.
 **/
typedef const struct QBDASHFrameSourceListener_ {
    /**
     * Method called when information about MP4 container is ready.
     *
     * @param[in] self_         listener handle
     * @param[in] frameSource   frame source handle
     * @param[in] info          information about MP4 media
     **/
    void (*infoReport)(SvObject self_,
                       QBDASHFrameSource frameSource,
                       QBMp4Info* info);

    /**
     * Method called when new MP4 frame is parsed.
     *
     * @param[in] self_         listener handle
     * @param[in] frameSource   frame source handle
     * @param[in] frameData     MP4 frame data
     **/
    void (*frameReport)(SvObject self_,
                        QBDASHFrameSource frameSource,
                        const struct QBDASHMP4FrameData* frameData);

    /**
     * Handle change of frame source's state.
     *
     * @param[in] self_         listener object handle
     * @param[in] frameSource   frame source handle
     * @param[in] state         frame source state
     * @param[in] initSegment   @c true if the state is for the init segment
     **/
    void (*stateChanged)(SvObject self_,
                         QBDASHFrameSource frameSource,
                         QBDASHFrameSourceState state,
                         bool initSegment);

    /**
     * Handle frame source's error.
     *
     * @param[in] self_         listener object handle
     * @param[in] frameSource   frame source handle
     * @param[in] errorType     error type
     **/
    void (*error)(SvObject self_,
                  QBDASHFrameSource frameSource,
                  QBDASHFrameSourceErrorType errorType);
} *QBDASHFrameSourceListener;

/**
 * Get runtime identification object representing
 * QBDASHFrameSourceListener interface.
 *
 * @return QBDASHFrameSourceListener interface
 **/
extern SvInterface
QBDASHFrameSourceListener_getInterface(void);

/**
 * @}
 **/

#endif // QBDASHFRAMESOURCELISTENER_H

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

#ifndef QBMP4PARSERENGINELISTENER_H_
#define QBMP4PARSERENGINELISTENER_H_

/**
 * @file QBMP4ParserEngineListener.h
 * @brief MP4 Parser Engine listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/MP4/QBMP4Parser/QBMP4ParserEngine.h>
#include <QBIPStreamClient/MP4/QBMP4Parser/QBMP4Data.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvPlayerKit/SvChbuf.h>

/**
 * @defgroup QBMP4ParserEngineListener MP4 Parser Engine listener interface
 * @{
 **/

/**
 * QBMP4ParserEngineListener interface.
 **/
typedef const struct QBMP4ParserEngineListener_ {
    /**
     * Method called when information about MP4 container is ready.
     *
     * @param[in] self_  listener handle
     * @param[in] engine MP4 parser engine handle
     * @param[in] info   information about MP4 media
     **/
    void (*infoReport)(SvObject self_,
                       QBMP4ParserEngine engine,
                       QBMp4Info *info);

    /**
     * Method called when new MP4 frame is parsed.
     *
     * @param[in] self_      listener handle
     * @param[in] engine     MP4 parser engine handle
     * @param[in] chb        parsed frame
     * @param[in] trackIndex index of track for which frame belongs (only in file mode)
     * @param[in] timescale  timescale value
     * @param[in] pts        presentation time stamp of frame (in file mode in 90 kHz units, in fragmented mode in timescale units)
     * @param[in] segmentPTS presentation time stamp of segment (available only in fragmented mode in timescale units), -1 if not available
     * @param[in] duration   duration of frame (only in fragmented mode)
     **/
    void (*frameReport)(SvObject self_,
                        QBMP4ParserEngine engine,
                        SvChbuf chb,
                        uint32_t trackIndex,
                        uint64_t timescale,
                        uint64_t pts,
                        int64_t segmentPTS,
                        uint64_t duration);

    /**
     * Method called when MP4 Parser Engine state is changed.
     *
     * @param[in] self_  listener handle
     * @param[in] engine MP4 parser engine handle
     * @param[in] state  state
     **/
    void (*stateChanged)(SvObject self_,
                         QBMP4ParserEngine engine,
                         QBMP4ParserEngineState state);

    /**
     * Method called when error occurred.
     *
     * @param[in] self_  listener handle
     * @param[in] engine MP4 parser engine handle
     * @param[in] error  error
     **/
    void (*error)(SvObject self_,
                  QBMP4ParserEngine engine,
                  QBMP4ParserEngineError error);
} *QBMP4ParserEngineListener;

/**
 * Get runtime identification object representing
 * MP4 Parser Engine listener interface.
 *
 * @return MP4 Parser Engine listener interface
 **/
extern SvInterface
QBMP4ParserEngineListener_getInterface(void);

/**
 * @}
 **/

#endif /* QBMP4PARSERENGINELISTENER_H_ */

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

#ifndef QBTELETEXTLINERECEIVER_H_
#define QBTELETEXTLINERECEIVER_H_

#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvType.h>
#include <stdint.h>
#include <SvPlayerManager/SvPlayerTask.h>

/**
 * @defgroup QBTeletextLineReceiver Teletext lines handler.
 * @{
 **/

typedef struct QBTeletextLineReceiver_ *QBTeletextLineReceiver;

/**
 * @class QBTeletextLineReceiverListener Teletext Lines listener interface
 */
typedef const struct QBTeletextLineReceiverListener_s {
    /**
     * Teletext flushed
     * @param self_ listener
     */
    void (*flush)(SvObject self_);

    /**
     * Timestamp (PTS) received
     * @param self_ listener
     * @param pts timestamp
     */
    void (*pts)(SvObject self, int64_t pts);

    /**
     * Teletext line received
     * @param self_ listener
     * @param line line data
     */
    void (*line)(SvObject self, const uint8_t *line);
} *QBTeletextLineReceiverListener;

SvInterface QBTeletextLineReceiverListener_getInterface(void);

/**
 * @brief Create new QBTeletextLineReceiver instance
 *
 * @param[in] playerTask player task
 * @param[in] pid teletext pid
 *
 * @return QBTeletextLineReceiver instance
 */
QBTeletextLineReceiver QBTeletextLineReceiverNew(SvPlayerTask playerTask, int pid);

/**
 * @brief Starts QBTeletextLineReceiver instance
 *
 * @param[in] self QBTeletextLineReceiver instance to start
 */
void QBTeletextLineReceiverStart(QBTeletextLineReceiver self);

/**
 * @brief Stops QBTeletextLineReceiver instance
 *
 * @param[in] self QBTeletextLineReceiver instance to stop
 */
void QBTeletextLineReceiverStop(QBTeletextLineReceiver self);

/**
 * @brief Adds listener
 *
 * @param[in] self QBTeletextLineReceiver instance
 * @param[in] listener listener to add
 */
void QBTeletextLineReceiverAddListener(QBTeletextLineReceiver self, SvObject listener);

/**
 * @brief Removes listener
 *
 * @param[in] self QBTeletextLineReceiver instance
 * @param[in] listener listener to remove
 */
void QBTeletextLineReceiverRemoveListener(QBTeletextLineReceiver self, SvObject listener);

/** @} */

#endif

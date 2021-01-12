/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef VIDEO_FORMAT_MONITOR_H_
#define VIDEO_FORMAT_MONITOR_H_

#include <SvFoundation/SvCoreTypes.h>
#include <QBViewport.h>
#include <fibers/c/fibers.h>


typedef struct QBVideoFormatListener_ {
    void (*formatChanged)(SvObject self_,
                          const QBViewportVideoInfo *videoInfo);
    /**
     * Video info updated.
     *
     * @param[in] self_     registered listener handle
     * @param[in] videoInfo current video format info
     */
    void (*infoUpdated)(SvObject self_,
                        const QBViewportVideoInfo *videoInfo);
} *QBVideoFormatListener;

extern SvInterface QBVideoFormatListener_getInterface(void);


typedef struct QBVideoFormatMonitor_ *QBVideoFormatMonitor;

/**
 * Create video format monitor.
 *
 * @param[in] viewport      viewport from which video will be monitored
 * @param[in] pollPeriod    poll period of monitored data
 * @return                  new video format monitor instance
 */
extern QBVideoFormatMonitor
QBVideoFormatMonitorCreate(QBViewport viewport,
                           unsigned int pollPeriod);

extern void
QBVideoFormatMonitorAddListener(QBVideoFormatMonitor self,
                                SvObject listener);

extern void
QBVideoFormatMonitorRemoveListener(QBVideoFormatMonitor self,
                                   SvObject listener);

/**
 * Get video format info of content that is currently played.
 *
 * @param[in] self          QBVideoFormatMonitor handle
 * @return                  current video info or @c NULL if video info haven't been found yet
 */
extern const QBViewportVideoInfo *
QBVideoFormatGetCurrentInfo(QBVideoFormatMonitor self);

#endif
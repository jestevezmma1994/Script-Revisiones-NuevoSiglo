/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSMPTETTPNGDECODER_H_
#define QBSMPTETTPNGDECODER_H_

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvInterface.h>
#include <CAGE/Core/SvBitmap.h>
#include <fibers/c/fibers.h>


// QBSMPTETTPngDecoderTask

typedef enum {
    QBSMPTETTPngDecoderTaskStatus_unknown,
    QBSMPTETTPngDecoderTaskStatus_failed,
    QBSMPTETTPngDecoderTaskStatus_canceled,
    QBSMPTETTPngDecoderTaskStatus_completed
} QBSMPTETTPngDecoderTaskStatus;

typedef struct QBSMPTETTPngDecoderTask_ *QBSMPTETTPngDecoderTask;

/**
 * Get the resulting bitmap
 *
 * @param[in]   self       handle to QBSMPTETTPngDecoderTask
 * @return      handle to bitmap object , @c NULL in case of error
 **/
SvBitmap
QBSMPTETTPngDecoderTaskGetBitmap(QBSMPTETTPngDecoderTask self);

/**
 * Get QBSMPTETTPngDecoder task status
 *
 * @param[in]   self       handle to QBSMPTETTPngDecoderTask
 * @return      QBSMPTETTPngDecoderTask status
 **/
QBSMPTETTPngDecoderTaskStatus
QBSMPTETTPngDecoderTaskGetStatus(QBSMPTETTPngDecoderTask self);


// QBSMPTETTPngDecoderListener

typedef struct QBSMPTETTPngDecoderListener_t {
    void (*taskCompleted)(SvObject self_, QBSMPTETTPngDecoderTask task);
} *QBSMPTETTPngDecoderListener;

/**
 * Get QBSMPTETTPngDecoder listener interface
 *
 * @return      handle to interface object, @c NULL in case of error
 **/
SvInterface
QBSMPTETTPngDecoderListener_getInterface(void);


// QBSMPTETTPngDecoder

typedef struct QBSMPTETTPngDecoder_ *QBSMPTETTPngDecoder;

/**
 * Create new QBSMPTETTPngDecoder object
 *
 * @param[in]   scheduler handle to scheduler that should be used
 * @return      handle to new QBSMPTETTPngDecoder, @c NULL in case of error
 **/
QBSMPTETTPngDecoder
QBSMPTETTPngDecoderCreate(SvScheduler scheduler);

/**
 * Add new QBSMPTETTPngDecoder task
 *
 * @param[in]   self       handle to QBSMPTETTPngDecoder
 * @param[in]   imageData  pointer to binary data of the image
 * @param[in]   listener   handle to listener for this particular task, may be NULL
 * @return      handle to newly created QBSMPTETTPngDecoderTask, @c NULL in case of error
 **/
QBSMPTETTPngDecoderTask
QBSMPTETTPngDecoderAddTask(QBSMPTETTPngDecoder self, SvData imageData, SvObject listener);

/**
 * Cancel QBSMPTETTPngDecoder task
 *
 * @param[in]   self   handle to QBSMPTETTPngDecoder
 * @param[in]   task   handle to task that should be canceled
 **/
void
QBSMPTETTPngDecoderCancelTask(QBSMPTETTPngDecoder self, QBSMPTETTPngDecoderTask task);

/**
 * Add QBSMPTETTPngDecoder listener
 *
 * @param[in]   self       handle to QBSMPTETTPngDecoder
 * @param[in]   listener   handle to QBSMPTETTPngDecoder listener
 * @return      positive integer value on success, @c negative value in case of error
 **/
int
QBSMPTETTPngDecoderAddListener(QBSMPTETTPngDecoder self, SvObject listener);

/**
 * Remove QBSMPTETTPngDecoder lisetener
 *
 * @param[in]   self       handle to QBSMPTETTPngDecoder
 * @param[in]   listener   handle to QBSMPTETTPngDecoder listener
 * @return      positive integer value on success, @c negative value in case of error
 **/
int
QBSMPTETTPngDecoderRemoveListener(QBSMPTETTPngDecoder self, SvObject listener);

#endif /* QBSMPTETTPNGDECODER_H_ */

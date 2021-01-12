/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_INPUT_THREAD_H_
#define QB_INPUT_THREAD_H_

/**
 * @file QBInputThread.h
 * @brief Input thread class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBInput/QBInputQueue.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBInputThread Input thread class
 * @ingroup QBInputCore
 *
 * Input thread class manages a @ref QBInputService running asynchronously in
 * a separate POSIX thread.
 *
 * @{
 **/

/**
 * Input thread class.
 * @class QBInputThread
 * @extends SvObject
 **/
typedef struct QBInputThread_ *QBInputThread;


/**
 * Create input thread.
 *
 * @memberof QBInputThread
 *
 * @param[out] errorOut error info
 * @return              created input thread, @c NULL in case of error
 **/
extern QBInputThread QBInputThreadCreate(SvErrorInfo *errorOut);

/**
 * Get input events queue filled with events by the input thread.
 *
 * @memberof QBInputThread
 *
 * @param[in] self      input thread handle
 * @param[out] errorOut error info
 * @return              input events queue, @c NULL in case of error
 **/
extern QBInputQueue QBInputThreadGetOutputQueue(QBInputThread self, SvErrorInfo *errorOut);

/**
 * Start input thread.
 *
 * @memberof QBInputThread
 *
 * @param[in] self      input thread handle
 * @param[out] errorOut error info
 **/
extern void QBInputThreadStart(QBInputThread self, SvErrorInfo *errorOut);

/**
 * Stop input thread.
 *
 * @memberof QBInputThread
 *
 * @param[in] self      input thread handle
 * @param[out] errorOut error info
 **/
extern void QBInputThreadStop(QBInputThread self, SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

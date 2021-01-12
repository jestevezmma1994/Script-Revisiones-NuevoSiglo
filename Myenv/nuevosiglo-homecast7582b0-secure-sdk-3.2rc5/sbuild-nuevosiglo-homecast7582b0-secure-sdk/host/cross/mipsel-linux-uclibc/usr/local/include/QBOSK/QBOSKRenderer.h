/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_OSK_RENDERER_H_
#define QB_OSK_RENDERER_H_

/**
 * @file QBOSKRenderer.h
 * @brief On Screen Keyboard bitmap renderer class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBOSK/QBOSKTypes.h>


/**
 * @defgroup QBOSKRenderer On Screen Keyboard bitmap renderer class
 * @ingroup QBOSKCore
 * @{
 **/

/**
 * Get runtime type identification object representing
 * OSK bitmap renderer class.
 *
 * @return QBOSKRenderer type identification object
 **/
extern SvType
QBOSKRenderer_getType(void);

/**
 * Create new OSK bitmap renderer.
 *
 * @param[out] errorOut error info
 * @return              created renderer, @c NULL in case of error
 **/
extern QBOSKRenderer
QBOSKRendererCreate(SvErrorInfo *errorOut);

/**
 * Start asynchronous rendering operations.
 *
 * @param[in] self      OSK renderer handle
 * @param[in] scheduler scheduler to be used for asynchronous rendering,
 *                      @c NULL to use default one
 * @param[out] errorOut error info
 **/
extern void
QBOSKRendererStart(QBOSKRenderer self,
                   SvScheduler scheduler,
                   SvErrorInfo *errorOut);

/**
 * Stop asynchronous rendering operations.
 *
 * @param[in] self      OSK renderer handle
 * @param[out] errorOut error info
 **/
extern void
QBOSKRendererStop(QBOSKRenderer self,
                  SvErrorInfo *errorOut);

/**
 * Get number of scheduled asynchronous rendering tasks
 * waiting to be finished.
 *
 * @param[in] self      OSK renderer handle
 * @return              number of pending tasks, @c -1 in case of error
 **/
extern int
QBOSKRendererGetTasksCount(QBOSKRenderer self);

/**
 * Schedule asynchronous rendering task.
 *
 * @param[in] self      OSK renderer handle
 * @param[in] task      rendering task handle
 * @param[out] errorOut error info
 **/
extern void
QBOSKRendererScheduleTask(QBOSKRenderer self,
                          QBOSKRenderTask task,
                          SvErrorInfo *errorOut);
/**
 * Cancel all tasks.
 *
 * @param[in] self       OSK renderer handle
 **/
extern void
QBOSKRendererCancelAllTasks(QBOSKRenderer self);

/**
 * @}
 **/


#endif

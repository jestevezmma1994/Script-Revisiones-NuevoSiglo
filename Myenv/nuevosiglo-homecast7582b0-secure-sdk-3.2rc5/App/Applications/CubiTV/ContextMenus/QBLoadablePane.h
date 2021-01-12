/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBLOADABLEPANE_H_
#define QBLOADABLEPANE_H_

/**
 * @file QBLoadablePane.h LoadablePane class
 **/

#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <main_decl.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakReference.h>
#include <fibers/c/fibers.h>
#include <stdbool.h>

/**
 * @defgroup QBLoadablePane LoadablePane class.
 * @{
 **/

/**
 * QBLoadablePaneState definition.
 **/
typedef enum QBLoadablePaneState_e {
    QBLoadablePaneStateCreated = 0, /// created successfully
    QBLoadablePaneStateInitialized, /// initialized successfully
    QBLoadablePaneStateStarted,     /// time out counter started
    QBLoadablePaneStateShown,       /// waitWidget was shown
    QBLoadablePaneStateStopped,     /// time out counter stopped
    QBLoadablePaneStateHidden,      /// waitWidget was hidden
    QBLoadablePaneStateTimeOut,     /// after time out state
    QBLoadabelPaneStateMax
} QBLoadablePaneState;

/**
 * QBLoadablePane class. Base class for objects that want to use loading widget.
 **/
struct QBLoadablePane_ {
    struct SvObject_ super_;

    QBContainerPane waitPane;
    SvWidget waitWidget;

    int timeOutMs;
    SvFiber fiber;
    SvFiberTimer timer;
    bool useTimeOut;

    SvWeakReference listener;

    QBLoadablePaneState state;
    int settingsCtx;
};
typedef struct QBLoadablePane_ *QBLoadablePane;

/**
 * Get runtime type identification object
 * representing QBLoadablePane class.
 * @return Loadable pane runtime type identification object
 **/
SvType
QBLoadablePane_getType(void);

/**
 * QBLodablePane creation and initialization. After calling this function there's
 * no need to call @see QBLoadablePaneInit.
 * If timeOutMs has value '0', LoadablePane won't start timeout counting.
 *
 * @param[in] ctxMenu    ContextMenu handle
 * @param[in] level      loadable pane level in ContextMenu
 * @param[in] timeOutMs  time out value in milliseconds
 * @param[in] appGlobals appGlobals handle
 * @param[out] errorOut  error info
 * @return               @c QBLoadablePane, if function end with success,
 *                       @c NULL, otherwise
 */
QBLoadablePane
QBLoadablePaneCreate(QBContextMenu ctxMenu, int level, int timeOutMs,
                     AppGlobals appGlobals, SvErrorInfo *errorOut);

/**
 * Init LoadablePane. If timeOutMs has value '0', LoadablePane won't start
 * timeout counting.
 *
 * @param[in] self       loadable pane handle
 * @param[in] ctxMenu    ContextMenu handle
 * @param[in] level      loadable pane level in ContextMenu
 * @param[in] timeOutMs  time out value in milliseconds
 * @param[in] appGlobals appGlobals handle
 * @param[out] errorOut  error info
 */
void
QBLoadablePaneInit(QBLoadablePane self, QBContextMenu ctxMenu, int level, int timeOutMs,
                   AppGlobals appGlobals, SvErrorInfo *errorOut);

/**
 * Set object that is pane listener. Listener need to implement
 * QBLoadablePaneListener interface.
 *
 * @param[in] self      loadable pane handle
 * @param[in] listener  object that is loadable pane listener
 * @param[out] errorOut error info
 * @return              @c true, if function ended successfully, @c false otherwise
 */
bool
QBLoadablePaneSetStateListener(QBLoadablePane self,
                               SvObject listener,
                               SvErrorInfo * errorOut);

/**
 * @}
 **/

#endif /* QBLOADABLEPANE_H_ */

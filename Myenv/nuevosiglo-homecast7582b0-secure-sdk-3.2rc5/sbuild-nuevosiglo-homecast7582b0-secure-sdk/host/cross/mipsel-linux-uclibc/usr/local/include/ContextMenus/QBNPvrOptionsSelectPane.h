/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_NPVR_OPTIONS_SELECT_PANE_H
#define QB_NPVR_OPTIONS_SELECT_PANE_H

/**
 * @file  QBNPvrOptionsSelectPane.h
 * @brief NPVR options select pane API
 **/

#include <ContextMenus/QBContextMenu.h>
#include <main_decl.h>

/**
 * @defgroup QBNPvrOptionsSelectPane NPVR options select pane class
 * @ingroup  CubiTV
 * @{
 **/
typedef struct QBNPvrOptionsSelectPane_* QBNPvrOptionsSelectPane;

/**
 * NPVR options select pane type to be displayed.
 **/
typedef enum {
    QBNPvrOptionsSelectPaneType_Premiere,
    QBNPvrOptionsSelectPaneType_SpaceRecovery
} QBNPvrOptionsSelectPaneType;

/**
 * Notify that NPVR options select pane has been approved (hidden).
 *
 * @param[in] self              handle to object implementing NPVR options select pane
 * @param[in] optionValue       NPVR recording option value
 **/
typedef void (*QBNPvrOptionsSelectPaneApproveCallback)(void* self, int optionValue);

/**
 * Create pane which allows to select the additional option defined by pane type when creating NPVR recording.
 *
 * @param[in] appGlobals    AppGlobals handle
 * @param[in] ctxMenu       context menu handle
 * @param[in] level         pane level, indexes starts with 1
 * @param[in] paneType      define pane type to create
 * @return                  new NPVR space recovery select pane instance
 **/
QBNPvrOptionsSelectPane QBNPvrOptionsSelectPaneCreate(
    AppGlobals appGlobals,
    QBContextMenu ctxMenu,
    int level,
    QBNPvrOptionsSelectPaneType paneType
    );

/**
 * Show NPVR options select pane.
 *
 * @param[in] self                  NPVR options select pane handle
 * @param[in] optionValue           NPVR recording option value
 **/
void QBNPvrOptionsSelectPaneShow(QBNPvrOptionsSelectPane self, int optionValue);

/**
 * Set NPVR options select pane callback.
 *
 * @param[in] self              NPVR options select pane handle
 * @param[in] callback          callback called after NPVR options select pane has been approved (hidden)
 * @param[in] callbackParam     user controlled first argument to callback
 **/
void QBNPvrOptionsSelectPaneSetApproveCallback(
    QBNPvrOptionsSelectPane self,
    QBNPvrOptionsSelectPaneApproveCallback callback,
    void* callbackParam
    );

/**
 * Translate NPVR recording option value of given NPVR options pane type to text description.
 *
 * @param[in] paneType      NPVR options select pane type
 * @param[in] optionValue   NPVR recording option value
 * @return                  text description of given NPVR recording option value
 **/
SvString TranslateOptionValue(QBNPvrOptionsSelectPaneType paneType, int optionValue);

/**
 * @}
 **/

#endif //QB_NPVR_OPTIONS_SELECT_PANE_H

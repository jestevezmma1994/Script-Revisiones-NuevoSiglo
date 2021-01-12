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

#ifndef QB_NPVR_MARGINS_PANE_H
#define QB_NPVR_MARGINS_PANE_H

/**
 * @file  QBNPvrMarginsPane.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief NPVR margins pane API
 **/

#include <ContextMenus/QBContextMenu.h>
#include <main_decl.h>

/**
 * @defgroup QBNPvrMarginsPane NPVR margins pane class
 * @ingroup  CubiTV
 * @{
 **/
typedef struct QBNPvrMarginsPane_* QBNPvrMarginsPane;

/**
 * Notify that NPVR margins pane has been approved (hidden).
 *
 * @param[in] self      handle to object implementing NPVR margins pane
 **/
typedef void (*QBNPvrMarginsPaneApproveCallback)(void* self);

/**
 * Create pane which allows to change NPVR start and end margins.
 *
 * @param[in] appGlobals        AppGlobals handle
 * @param[in] ctxMenu           context menu handle
 * @param[in] level             pane level, indexes starts with 1
 * @return                      new NPVR margins pane instance
 **/
QBNPvrMarginsPane QBNPvrMarginsPaneCreate(
    AppGlobals appGlobals,
    QBContextMenu ctxMenu,
    int level
    );

/**
 * Show NPVR margins pane.
 *
 * @param[in] self      NPVR margins pane handle
 **/
void QBNPvrMarginsPaneShow(QBNPvrMarginsPane self);

/**
 * Set NPVR margins pane callback.
 *
 * @param[in] self              NPVR margins pane handle
 * @param[in] callback          callback called after NPVR margins pane has been approved (hidden)
 * @param[in] callbackParam     user controlled first argument to callback
 **/
void QBNPvrMarginsPaneSetApproveCallback(QBNPvrMarginsPane self, QBNPvrMarginsPaneApproveCallback callback, void* callbackParam);

/**
 * @}
 **/

#endif //QB_NPVR_MARGINS_PANE_H

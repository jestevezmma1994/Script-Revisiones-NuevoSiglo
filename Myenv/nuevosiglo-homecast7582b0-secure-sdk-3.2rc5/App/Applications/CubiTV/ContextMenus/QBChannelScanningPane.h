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

#ifndef QBCHANNELSCANNINGPANE_H_
#define QBCHANNELSCANNINGPANE_H_

/**
 * @file QBChannelScanningPane.h QBChannelScanningPane class
 **/

#include <main_decl.h>
#include <ContextMenus/QBContextMenu.h>
#include <Services/core/QBTextRenderer.h>


/**
 * QBChannelScanningPane class. Class describing QBChannelScanning pane.
 **/
typedef struct QBChannelScanningPane_t *QBChannelScanningPane;

/**
 * QBChannelScanningPaneParams class. Class describing parameters used to initialize pane.
 **/
struct QBChannelScanningPaneParams {
    SvApplication app;
    SvScheduler scheduler;
    QBTextRenderer textRenderer;
};

/**
 * Get runtime type identification object
 * representing QBChannelScanningPane class.
 * @return QBChannelScanningPane runtime type identification object
 **/
SvType QBChannelScanningPane_getType(void);

/**
 * Initialization of QBChannelScanningPane object.
 *
 * @param[in] self      QBChannelScanningPane handle
 * @param[in] params    QBChannelScanningPaneParams handle
 * @param[in] ctxMenu   ContextMenu handle
 * @param[in] itemNamesFilename  json file name containing item description
 */
void QBChannelScanningPaneInit(QBChannelScanningPane self, struct QBChannelScanningPaneParams* params, QBContextMenu ctxMenu, SvString itemNamesFilename);

/**
 * Initialization of QBChannelScanningPane object.
 *
 * @param[in] settings          settings file name containing pane visual description
 * @param[in] itemNamesFilename json file name containing item description
 * @param[in] params            QBChannelScanningPaneParams handle
 * @param[in] ctxMenu           ContextMenu handle
 * @return                      new QBChannelScanningPane instance
 */
QBChannelScanningPane QBChannelScanningPaneCreateFromSettings(const char *settings, const char *itemNamesFilename, struct QBChannelScanningPaneParams* params, QBContextMenu ctxMenu);

#endif /* QBCHANNELSCANNINGPANE_H_ */

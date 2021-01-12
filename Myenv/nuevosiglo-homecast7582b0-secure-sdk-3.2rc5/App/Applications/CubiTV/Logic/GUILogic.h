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

#ifndef GUI_LOGIC_H_
#define GUI_LOGIC_H_

#include <main_decl.h>
#include <QBWindowContext.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBDataModel3/QBActiveTree.h>

typedef struct QBGUILogic_t *QBGUILogic;

/**
 * Get runtime type identification object representing QBGUILogic type.
 *
 * @return QBGUILogic runtime type identification object
**/
SvType QBGUILogic_getType(void);

QBGUILogic
QBGUILogicCreate(AppGlobals appGlobals) __attribute__((weak));

void
QBGUILogicStart(QBGUILogic self) __attribute__((weak));

void
QBGUILogicStop(QBGUILogic self) __attribute__((weak));

QBWindowContext
QBGUILogicPrepareEPGContext(QBGUILogic self,
                            SvString epgView,
                            SvString playlist) __attribute__((weak));

void QBGUILogicMenuBarItemControllerRegisterCallbacks(QBGUILogic self, SvObject menuBarItemController) __attribute__((weak));
void QBGUILogicPVRMenuContextRegisterCallbacks(QBGUILogic self, SvObject pvrMenuHandler) __attribute__((weak));
void QBGUILogicRemiderEditorPaneRegisterCallbacks(QBGUILogic self, SvObject reminderEditorPane) __attribute__((weak));
void QBGUILogicMiniChannelListRegisterCallbacks(QBGUILogic self, SvObject miniChannelList) __attribute__((weak));
void QBGUILogicContentSideMenuRegisterCallbacks(QBGUILogic self, SvObject contentSideMenu) __attribute__((weak));
void QBGUILogicChannelMenuControllerRegisterCallbacks(QBGUILogic self, SvObject channelController_) __attribute__((weak));

QBWindowContext
QBGUILogicPrepareVoDContext(QBGUILogic self,
                            SvString vodView,
                            SvObject path,
                            SvObject provider,
                            SvString serviceId,
                            SvString serviceName,
                            bool leaf,
                            QBActiveTree externalTree) __attribute__((weak));

/**
 * Create Background Widget.
 *
 * This is a method that creates CCBackgroundWidget with size of window.
 *
 * @param[in] self                      GUI Logic
 * @param[in] windowName                Window Name
 * @param[out] *errorOut                error info
 * @return                              created widget, @c NULL in case of error
 **/
SvWidget QBGUILogicCreateBackgroundWidget(QBGUILogic self, const char *windowName, SvErrorInfo *errorOut) __attribute__((weak));

#endif // GUI_LOGIC_H_

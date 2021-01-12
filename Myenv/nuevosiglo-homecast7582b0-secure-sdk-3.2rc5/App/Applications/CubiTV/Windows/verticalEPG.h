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

#ifndef VERTICALEPG_H_
#define VERTICALEPG_H_

/**
 * @file verticalEPG.h Vertical Electronic Program Guide window context API
 **/

#include <ContextMenus/QBContextMenu.h>
#include <main_decl.h>
#include <QBWindowContext.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <sys/time.h>

typedef enum {
    QBVerticalEPGSideMenuType_Full = 0,
    QBVerticalEPGSideMenuType_Stop,
    QBVerticalEPGSideMenuType_NPvrKeyword,
    QBVerticalEPGSideMenuType_NPvrSeries,
    QBVerticalEPGSideMenuType_SearchOnly
} QBVerticalEPGSideMenuType;

/**
 * Create Vertical EPG window context.
 *
 * @param[in] appGlobals     AppGlobals handle
 *
 * @return created window context or @c NULL in case of error
**/
QBWindowContext QBVerticalEPGContextCreate(AppGlobals appGlobals);

/**
 * Get runtime type identification object representing QBVerticalEPGWindow class.
 *
 * @return VerticalEPGWindow class
 **/
SvType VerticalEPGWindow_getType(void);

/**
 * Shows side menu with options and sets focus on an option with given id.
 *
 * @param[in] self_ EPG context handle
 * @param[in] id    id of the option on which the side menu's focus will be set
 * @param[in] type  type of the side menu
 **/
void QBVerticalEPGShowSideMenu(QBWindowContext self_, SvString id, QBVerticalEPGSideMenuType type);

/**
 * Shows side menu with options and sets focus on an option on given index.
 *
 * @param[in] self_  EPG context handle
 * @param[in] idx   index on which the side menu's focus will be set
 * @param[in] type  type of the side menu
 **/
void QBVerticalEPGShowSideMenuOnIndex(QBWindowContext self_, size_t idx, QBVerticalEPGSideMenuType type);

/**
 * Is any kind of side menu is shown in EPG vertical window.
 *
 * @param [in] self_     EPG vertical window context handle
 *
 * @return  @c true if side menu is shown otherwise @c false
 **/
bool QBVerticalEPGIsSideMenuShown(QBWindowContext self_);

/**
 * Hide any kind of side menu current displayed in EPG vertical window.
 *
 * @param [in] epg_  EPG vertical window context handle
 **/
void QBVerticalEPGHideSideMenu(QBWindowContext epg_);
void QBVerticalEPGShowExtendedInfo(QBWindowContext self_);

/**
* Set focus in vertical EPG window on specified TV channel in specified time.
*
* @param [in] self_         EPG vertical window context handle
* @param [in] channelID     TV channel ID
* @param [in] activeTime    time to set. If you pass @c 0 the current time will be set
*/
void QBVerticalEPGSetFocus(QBWindowContext self_, SvTVChannel channelID, time_t activeTime);
void QBVerticalEPGGetActiveEvent(QBWindowContext self_, SvEPGEvent *event);
/**
 * Retreive sidemenu context from Vertical EPG Context
 *
 * @param[in] epg_ Vertical EPG context
 * @return sidemenu context
 */
QBContextMenu QBVerticalEPGGetContextMenu(QBWindowContext epg_);
/**
 * Get current focused channel
 *
 * @param[in] self_ Vertical EPG Context
 * @return current channel
 */
SvTVChannel QBVerticalEPGGetChannel(QBWindowContext self_);
void QBVerticalEPGShowRecord(QBWindowContext self_);
bool QBVerticalEPGStopRecording(QBWindowContext self_);

#endif /* VERTICALEPG_H_ */

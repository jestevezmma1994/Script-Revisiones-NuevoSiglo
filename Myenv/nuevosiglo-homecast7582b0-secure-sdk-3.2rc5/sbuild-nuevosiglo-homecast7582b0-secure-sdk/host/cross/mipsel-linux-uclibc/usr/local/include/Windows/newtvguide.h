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
#ifndef NEWTVGUIDE_H_
#define NEWTVGUIDE_H_

#include <ContextMenus/QBContextMenu.h>
#include <QBWindowContext.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>

typedef enum {
    NewTVGuideSideMenuType_Full = 0,
    NewTVGuideSideMenuType_Stop,
    NewTVGuideSideMenuType_NPvrKeyword,
    NewTVGuideSideMenuType_NPvrSeries,
    NewTVGuideSideMenuType_SearchOnly,
} NewTVGuideSideMenuType;

QBWindowContext svNewTVGuideContextCreate(AppGlobals appGlobals);
SvType EPGContext_getType(void);

bool svNewTVGuideIsSideMenuShown(QBWindowContext epg_);

/**
 * Shows side menu with options and sets focus on an option with given id.
 *
 * @param[in] epg_  EPG context handle
 * @param[in] id    id of the option on which the side menu's focus will be set
 * @param[in] type  type of the side menu
 **/
void svNewTVGuideShowSideMenu(QBWindowContext epg_, SvString id, NewTVGuideSideMenuType type);

/**
 * Shows side menu with options and sets focus on an option on given index.
 *
 * @param[in] epg_  EPG context handle
 * @param[in] idx   index on which the side menu's focus will be set
 * @param[in] type  type of the side menu
 **/
void svNewTVGuideShowSideMenuOnIndex(QBWindowContext epg_, size_t idx, NewTVGuideSideMenuType type);
void svNewTVGuideHideSideMenu(QBWindowContext epg_);
void svNewTVGuideSetDefaultPosition(QBWindowContext epg_);
void svNewTVGuideShowExtendedInfo(QBWindowContext epg_);
void svNewTVGuideShowRecord(QBWindowContext epg_);
/* Stops recording (if any) for active EPG event.
 *  Uncompleted recording is stopped.
 *  Scheduled recording is removed.
 */
bool svNewTVGuideStopRecording(QBWindowContext epg_);
/**
 * Similar to svNewTVGuideSetChannel() but more general:
 * parses set-focus event, extracts channel to be set and 'saves' it.
 * Called on entering EPG Context (horizontal or vertical).
 * @param[in] epg_ EPG context
 * @param[in] event Event passed on EPG Context enter. Can be NULL.
 */
void svNewTVGuideSetFocus(QBWindowContext epg_, SvEPGEvent event);
/**
 * 'Saves' channel passed in second argument to EPG (epg_).
 * Updates activeChannel in EPG (type EPGContext_t).
 * @param[in] epg_ EPG context
 * @param[in] channel Channel to be saved in epg_ context
 */
void svNewTVGuideSetChannel(QBWindowContext epg_, SvTVChannel channel);
void svNewTVGuideGetActiveEvent(QBWindowContext epg_, SvEPGEvent *event);
SvTVChannel svNewTVGuideGetFocus(QBWindowContext epg_);
SvTVChannel svNewTVGuideGetChannel(QBWindowContext epg_);
SvString SvEPGEventGetDurationTime(SvEPGEvent event, const char *timeFormat);
/**
 * Retreive sidemenu context from EPG Context
 * @param[in] epg_ EPG context
 * @return sidemenu context
 */
QBContextMenu svNewTVGuideGetContextMenu(QBWindowContext epg_);

void svNewTVGuideSetPlaylist(QBWindowContext epg_, SvGenericObject playlist);
SvGenericObject svNewTVGuideGetCurrentPlaylist(QBWindowContext epg_);
#endif

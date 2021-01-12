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

#ifndef QBREMINDEREDITORPANEINTERNAL_H_
#define QBREMINDEREDITORPANEINTERNAL_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBExtendedInfoPane.h>
#include <ContextMenus/QBNPvrRecordingEditPane.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <CUIT/Core/types.h>
#include <Services/QBPVRConflictResolver.h>
#include <NPvr/QBnPVRRecording.h>
#include "QBReminderEditorPane.h"
#include <stdbool.h>

typedef enum {
    QBReminderEditorPanePlaybackType_Normal = 0,
    QBReminderEditorPanePlaybackType_StartOver,
    QBReminderEditorPanePlaybackType_StartCatchup,
    QBReminderEditorPanePlaybackType_WatchInCUTV,
    QBReminderEditorPanePlaybackType_ContinuousInCUTV
} QBReminderEditorPanePlaybackType;

typedef struct QBReminderEditorPaneCallbacks_s {
    void (*init)(void *self_, SvGenericObject reminderEditorPane, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel,
            SvEPGEvent event, SvString itemNamesFilename);
}*QBReminderEditorPaneCallbacks;


struct QBReminderEditorPane_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvEPGManager epgManager;
    QBContextMenu contextMenu;
    SvWidget sidemenu;

    QBBasicPane options;
    SvTVChannel channel;
    SvEPGEvent event;
    QBnPVRRecording currentNPvrRec;

    bool shown;
    bool playFromTheBeginning;
    QBReminderEditorPanePlaybackType playbackType;

    int settingsCtx;

    SvWidget errorLabel;
    SvWidget parentalPopup;

    QBPVRConflictResolver resolver;

    QBExtendedInfoPane extendedInfo;
    void *callbackData;
    struct QBReminderEditorPaneCallbacks_s callbacks;

    QBNPvrRecordingEditPane seriesPane;
    QBNPvrRecordingEditPane recordOnePane;

    QBWindowContext searchResults; ///< context managing search results or lack thereof
};

void QBReminderEditorPaneSetCallbacks(SvGenericObject self_, void *callbackData, QBReminderEditorPaneCallbacks callbacks);
void QBReminderEditorPaneInitImplementation(QBReminderEditorPane self, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel, SvEPGEvent event, SvString itemNamesFilename);


#endif // QBREMINDEREDITORPANEINTERNAL_H_

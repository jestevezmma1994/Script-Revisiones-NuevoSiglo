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


#ifndef QBFAVORITESEDITORPANEINTERNAL_H_
#define QBFAVORITESEDITORPANEINTERNAL_H_

#include <CUIT/Core/types.h>
#include <QBDataModel3/QBActiveArray.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBListPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <stdbool.h>
#include <main_decl.h>

struct QBFavoritesEditorPane_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvEPGManager epgManager;
    QBContextMenu contextMenu;
    SvWidget sideMenu;

    QBBasicPane options;
    SvTVChannel channel;

    bool shown;

    int settingsCtx;

    SvHashTable tickedChannels;
    QBListPane channelSelectionPane;

    SvString listID;
    SvString listName;

    QBActiveArray sortDataSource;
    QBListPane sortingPane;

    SvString addRemoveListType; ///< Type of channels the currently edited channel list consists of, e.g. TVChannels or RadioChannels
};

#endif // QBFAVORITESEDITORPANEINTERNAL_H_

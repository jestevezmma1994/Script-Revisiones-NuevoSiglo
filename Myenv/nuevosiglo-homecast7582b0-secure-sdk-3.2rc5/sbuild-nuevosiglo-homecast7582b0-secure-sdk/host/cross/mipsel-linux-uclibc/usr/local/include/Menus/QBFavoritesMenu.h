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

#ifndef QB_FAVORITES_MENU_H_
#define QB_FAVORITES_MENU_H_

/**
 * @file QBFavoritesMenu.h Favorites Menu class API
 **/

#include <main_decl.h>
#include <CUIT/Core/types.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreePathMap.h>


/**
 * Add new channel list to favorites menu.
 *
 * @param [in] menuBar          menu bar widged handle
 * @param [in] appGlobals       AppGlobals handle
 * @param [in] channelsList     new channel list
 * @param [in] nodeID           id menu where new list should be added, if @c NULL add to default favorites menu
 * @param [in] position         position in menu where new list shoud be added, if position is less than 0 then channel list will be added on the last position
 * @param [in] extended         flag indicate if additional node with EPG shoud be added
 */
extern void
QBFavoritesMenuAddChannelsList(SvWidget menuBar, AppGlobals appGlobals,
                               SvObject channelsList, SvString nodeID, int position, bool extended);

extern void
QBFavoritesMenuRemoveChannelsList(QBActiveTree menuTree, SvWidget menuBar,
                                  SvString listId);

/**
 * Rename channel list.
 *
 * @param [in] menuTree         QBActiveTree handle
 * @param [in] channelListID    channel list id
 * @param [in] channelListName  new name of channel list
 */
void
QBFavoritesMenuRenameChannelsList(QBActiveTree menuTree, SvString channelListID, SvString channelListName);

extern void
QBFavoritesMenuRegister(SvWidget menuBar,
                        QBTreePathMap pathMap,
                        AppGlobals appGlobals);


#endif

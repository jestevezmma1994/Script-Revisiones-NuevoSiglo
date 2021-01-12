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

#ifndef SEARCHRESULTS_H_
#define SEARCHRESULTS_H_

#include <SvPlayerKit/SvEPGEvent.h>
#include <QBWindowContext.h>
#include <ContextMenus/QBContextMenu.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>

void QBSearchResultsSetDataSource(QBWindowContext ctx_, SvGenericObject dataSource);
QBWindowContext QBSearchResultsContextCreate(AppGlobals appGlobals);

/**
 * Shows side menu with options and sets focus on an option with given id.
 *
 * @param[in] ctx_  search results context handle
 * @param[in] id    id of the option on which the side menu's focus will be set
 **/
void QBSearchResultsContextShowSideMenu(QBWindowContext ctx_, SvString id);

/**
 * Shows side menu with options and sets focus on an option on given index.
 *
 * @param[in] ctx_  search results context handle
 * @param[in] idx   index on which the side menu's focus will be set
 **/
void QBSearchResultsContextShowSideMenuOnIndex(QBWindowContext ctx_, size_t idx);
bool QBSearchResultsContextIsSideMenuShown(QBWindowContext ctx_);
void QBSearchResultsContextHideSideMenu(QBWindowContext ctx_);
void QBSearchResultsContextShowExtendedInfo(QBWindowContext ctx_);
void QBSearchResultsGetActiveEvent(QBWindowContext ctx_, SvEPGEvent *event);

/**
 * Executes the search. Hides given context menu and:
 *  - brings it back, if no results were found;
 *  - opens new context with results, otherwise.
 *
 * @param[in] ctx_              search results context handle
 * @param[in] menuToBringBack   context menu handle to be brought back in case of no results
 **/
void QBSearchResultsContextExecute(QBWindowContext ctx_, QBContextMenu menuToBringBack);

#endif /* SEARCHRESULTS_H_ */

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCONTENTSIDEMENU_H_
#define QBCONTENTSIDEMENU_H_

#include <SvFoundation/SvCoreTypes.h>
#include <ContextMenus/QBBasicPane.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <ContextMenus/QBInnov8onLoadablePane.h>
#include <QBMWClient/QBMWClientEventReporter.h>
#include <main_decl.h>

typedef enum {
    QBContentSideMenuImmediateAction_none,          ///< no action performed automatically
    QBContentSideMenuImmediateAction_play,          ///< play movie if user is entitled
    QBContentSideMenuImmediateAction_rent,          ///< rent movie if user is not entitled
    QBContentSideMenuImmediateAction_rentOrPlay     ///< play or rent movie depending on the curent entitlement status
} QBContentSideMenuImmediateAction;                 ///< automatic action executed when offer data is refreshed

typedef struct QBContentSideMenu_t* QBContentSideMenu;
typedef void (*QBContentSideMenuPaneInitCallback)(void* owner, QBContentSideMenu self, QBInnov8onLoadablePane pane);

/**
 * Create instance of QBContentSideMenu.
 *
 * @param[in] appGlobals        appGlobals handle
 * @param[in] optionFile        option file
 * @param[in] accessDomain      domain for access manager
 * @return                      created object, @c NULL in case of error
 **/
QBContextMenu
QBContentSideMenuCreate(AppGlobals appGlobals, SvString optionFile, SvString accessDomain);

/**
 * Set event reporter. Event reporter will be used to send notification about occurrred events such as
 * play, rent, play trailer.
 *
 * @param[in] self              content side menu handle
 * @param[in] eventReporter     event reporter
 **/
void
QBContentSideMenuSetEventReporter(QBContentSideMenu self, SvObject eventReporter);

void
QBContentSideMenuInitDefault(QBContentSideMenu self, SvDBRawObject product);

void
QBContentSideMenuSetCallbacks(QBContentSideMenu self,
                              void* owner,
                              QBContentSideMenuPaneInitCallback paneInit);

/**
 * Setter for contentInfo in QBContentSideMenu class.
 *
 * @param[in] self Content side menu handle
 * @param[in] contentInfo_ content to set
 * @param[in] categoryName content cateogry name
 */
void
QBContentSideMenuSetContentInfo(QBContentSideMenu self, SvGenericObject contentInfo_, SvString categoryName, SvString recomendedFrom); // AMERELES #2131 tracking de navegación VOD en STBs híbridos
//QBContentSideMenuSetContentInfo(QBContentSideMenu self, SvGenericObject contentInfo_);

/**
 * Decides if object which data will be refreshed need to be updated.
 * This function is useful if the data provider does not provide rental information.
 * That is the case with Digitalsmmiths - we get all the movie info from their backend
 * expect for the rental info and content urls.
 *
 * @param [in] self             content side menu handle
 * @param [in] refreshObject    @c true if the object needs to be updated and @c false otherwise
 **/
void
QBContentSideMenuSetRefreshObject(QBContentSideMenu self, bool refreshObject);

/**
 * Set the action to be executed as soon as the movie rental state is known.
 *
 * @param [in] self     content side menu handle
 * @param [in] action   action to perform when offers data has been refreshed
 **/
void
QBContentSideMenuSetImmediateAction(QBContentSideMenu self, QBContentSideMenuImmediateAction action);

#endif /* QBCONTENTSIDEMENU_H_ */

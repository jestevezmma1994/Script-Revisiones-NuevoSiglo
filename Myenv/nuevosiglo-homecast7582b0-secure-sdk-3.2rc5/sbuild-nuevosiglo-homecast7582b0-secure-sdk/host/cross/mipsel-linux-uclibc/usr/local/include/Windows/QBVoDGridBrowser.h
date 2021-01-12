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

#ifndef QB_VOD_GRID_BROWSER_H_
#define QB_VOD_GRID_BROWSER_H_

/**
 * @file QBVoDGridBrowser.h VoD grid browser.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBWindowContext.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvSet.h>
#include <SvFoundation/SvString.h>
#include <QBDataModel3/QBActiveTree.h>
#include <main_decl.h>
#include <stdbool.h>

/**
 * @defgroup QBVoDGridBrowser VoD grid browser class.
 * @ingroup CubiTV
 * @{
 *
 * A window class designed to show VoD assets or categories in a grid.
 * The window provides top bar with buttons, opt menu,
 * fucesed elements detail, and the assets grid.
 * The window is capable of connecting to a specific node in a content tree.
 * When the window connects to a node then all children of this node
 * become represented as rows, while grandchildren gets inserted in columns of those rows.
 * If a node that the view is connected to is a leaf category (has only children and no grandchildren)
 * then the view will show the leaf category as an only row and its children as columns in this row.
 **/
typedef struct QBVoDGridBrowserContext_ *QBVoDGridBrowserContext;

/**
 * Get runtime type identification object representing
 * VoD grid browser type.
 *
 * @return      QBVoDGridBrowser class type
 **/
SvType
QBVoDGridBrowserContext_getType(void);

/**
 * Available types of presenting movie details
 **/
typedef enum {
    QBVoDGridBrowserDetailsScreen_Default, ///< default movie details screen
    QBVoDGridBrowserDetailsScreen_ColumbusDS ///< movie details designed for Columbus Digitalsmiths integration
} QBVoDGridBrowserDetailsScreen;

/**
 * Settings class for deciding on the behaviour of VOD grid window
 **/
typedef struct QBVoDGridBrowserSettings_ {
    struct SvObject_ super; ///< superclass object
    bool useSideMenu;   ///< if @c true the side menu will be shown after pressing @c OPT button, if @c false the top navigation bar will be focused
    bool searchInNewTab;    ///< indicates if search should be performed in a separate tab or in the first results row.
    bool skipSearchDialog;      ///< indicates if dialogs should popup when performing search
    QBVoDGridBrowserDetailsScreen detailsScreen;    ///< what type of details screen should be used
    ssize_t defaultButtonIndex; ///< which button will be focused by default when focusing navigation bar. If not specified the first button is focussed by default
    SvString settingsFile; ///< which file use for settings. If not specified the default settings file will be used
    bool showPopupInEmptyCategory; ///< indicates if popup will be displayed when user enters into empty category
    bool refreshObject; ///< indicates if object data should be refreshed when asking MW for vod offers
    bool enableKeyShortcuts; ///< indicates if RC key shortcuts should be enabled
    bool moviesButtonMimicsVodButton; ///< indicates what happens when user presses "Movies" top button @c true means it is equivalent to pressing VOD on RC, @c false means pushing next context
    bool moveToRootOnMoviesButton; ///<indicates whether the top "Movies" button always moves to root of tree (@c true), or just makes sure main tree is browsed(@c false)
} * QBVoDGridBrowserSettings;

/**
 * Create settings object
 *
 * @return      new instance of settings object
 **/
QBVoDGridBrowserSettings QBVoDGridBrowserSettingsCreate(void);

/**
 * Create a VoD grid browser window and connect it to a specific tree node.
 *
 * @param[in] appGlobals            application global data handle
 * @param[in] path                  path to the node to which the window should connect to
 * @param[in] provider              the data provider implementing @ref QBContentProvider
 * @param[in] serviceId             id of service providing data for grid
 * @param[in] serviceName           name of service providing data for grid
 * @param[in] externalTree          tree containing node to which the @c path points
 * @param[in] unlockedCategories    set of categories the window should not block (can be @c NULL).
 * @param[in] settings              detail settings for grid window or @c NULL. If @c NULL the defaults will be used for settings.
 * @return                          new QBVoDGridBrowser window
 **/
extern QBWindowContext
QBVoDGridBrowserContextCreate(AppGlobals appGlobals, SvObject path,
                              SvObject provider, SvString serviceId,
                              SvString serviceName, QBActiveTree externalTree,
                              SvSet unlockedCategories,
                              QBVoDGridBrowserSettings settings);

/**
 * Reset context state so that it displays the initial VOD window
 *
 * @param[in] self  context handle
 **/
void
QBVoDGridBrowserContextReset(QBVoDGridBrowserContext self);

/**
 * @}
 **/

#endif

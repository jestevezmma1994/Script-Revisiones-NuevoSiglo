/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSATELLITESETUPEDITORPANE_H_
#define QBSATELLITESETUPEDITORPANE_H_

/**
 * @file QBSatelliteSetupEditorPane.h pane with satellite setup options
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <ContextMenus/QBContextMenu.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h> // IWYU pragma: keep
#include <main_decl.h>

/**
  * Structure describing context menu in Satellite Setup.
  *
  **/
typedef struct QBSatelliteSetupEditorPane_ * QBSatelliteSetupEditorPane;

/**
  * Create context menu used for editing satellites
  *
  * @param[in] settings     settings file to be used
  * @param[in] app          application handler
  * @param[in] ctxMenu      context menu in which pane has to be created
  * @param[in] widgetName   name of widget
  * @param[in] satellitesList       list of all available satellites
  * @param[in] selectedSatellite    satellite to be edited in some of the options
  * @param[in] singleSatelliteMode  indicates whether options for editing satellite list should be displayed
  * @param[out] errorOut     error(s) that happened during creation
  * @return                  resulting menu or NULL in case of error
  **/
QBSatelliteSetupEditorPane
QBSatelliteSetupEditorPaneCreateFromSettings(const char *settings,
                                             SvApplication app,
                                             QBContextMenu ctxMenu,
                                             SvString widgetName,
                                             SvArray satellitesList,
                                             int selectedSatellite,
                                             bool singleSatelliteMode,
                                             SvErrorInfo *errorOut);

/**
  * Sets listener
  *
  * @param[in] self         menu object
  * @param[in] listener     listener
  * @return                  0 on success, negative value on failure
  **/
int
QBSatelliteSetupEditorPaneSetListener(QBSatelliteSetupEditorPane self,
                                      SvGenericObject listener);

#endif /* QBSATELLITESETUPEDITORPANE_H_ */

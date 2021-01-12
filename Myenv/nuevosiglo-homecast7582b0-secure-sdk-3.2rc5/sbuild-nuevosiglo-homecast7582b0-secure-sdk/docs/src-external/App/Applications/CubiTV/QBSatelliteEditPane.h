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

#ifndef QBSATELLITEEDITPANE_H_
#define QBSATELLITEEDITPANE_H_

/**
 * @file QBSatelliteEditPane.h pane with satellite edition
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <ContextMenus/QBContextMenu.h>
#include <QBDVBSatelliteDescriptor.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>

/**
  * Simplified satellite descriptor, used for storing edited satellite
  *
  **/
typedef struct QBBasicSatelliteData_s {
    /// name of satellite (without degrees part)
    SvString name;
    /// hemisphere that satellite is over
    SvString hemisphere;
    /// degrees that satellite is at
    int      deg;
} QBBasicSatelliteData_;
typedef QBBasicSatelliteData_ * QBBasicSatelliteData;

/**
  * Satellite edition pane class.
  *
  **/
typedef struct QBSatelliteEditPane_ * QBSatelliteEditPane;

/**
 * Notify that satellite edition has finished.
 *
 * @param[in] self      object that has to retrieve data
 * @param[in] pane      satellite edition pane
 * @param[in] satelliteData    data of new/edited satellite
 **/
typedef void (*QBSatelliteEditCallback)(void *self_,
                                        QBSatelliteEditPane pane,
                                        QBBasicSatelliteData satelliteData);

/**
  * Get current level inside a menu
  *
  * @param[in] self         satellite edition pane
  * @return                  integer with the level
  **/
int
QBSatelliteEditPaneGetLevel(QBSatelliteEditPane self);

/**
  * Create transponder edition pane
  *
  * @param[in] ctxMenu            context menu in which pane has to be created
  * @param[in] app                CUIT application handle
  * @param[in] level              level inside a menu
  * @param[in] editedSatellite    satellite to be edited or NULL for new
  * @param[in] satellitesList     list of all available satellites
  * @param[in] callback           callback
  * @param[in] callbackData       additional data for callback
  * @param[out] errorOut          error(s) that happened during creation
  * @return                       resulting pane or NULL in case of error
  **/
QBSatelliteEditPane
QBSatelliteEditPaneCreate(QBContextMenu ctxMenu,
                          SvApplication app,
                          int level,
                          QBDVBSatelliteDescriptor editedSatellite,
                          SvArray satellitesList,
                          QBSatelliteEditCallback callback,
                          void *callbackData,
                          SvErrorInfo *errorOut);

#endif /* QBSATELLITEEDITPANE_H_ */

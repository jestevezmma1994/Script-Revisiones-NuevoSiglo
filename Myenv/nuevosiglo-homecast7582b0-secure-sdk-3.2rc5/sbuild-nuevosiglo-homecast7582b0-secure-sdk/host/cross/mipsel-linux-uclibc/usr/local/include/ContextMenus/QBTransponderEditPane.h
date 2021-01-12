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

#ifndef QBTRANSPONDEREDITPANE_H_
#define QBTRANSPONDEREDITPANE_H_

/**
 * @file QBTransponderEditPane.h pane with transponder edition
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <ContextMenus/QBContextMenu.h>
#include <QBDVBSatelliteDescriptor.h>
#include <QBTunerTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <main_decl.h>

/**
  * Simplified transponder data, used for storing edited transponder
  *
  **/
struct QBBasicTransponderData_ {
    /// frequency
    int freq;
    /// symbol rate
    int symbolRate;
    /// polarization
    QBTunerPolarization polarization;
    /// modulation
    QBTunerModulation modulation;
};
typedef struct QBBasicTransponderData_ * QBBasicTransponderData;

/**
  * Transponder edition pane class.
  *
  **/
typedef struct QBTransponderEditPane_ * QBTransponderEditPane;

/**
 * Notify that transponder edition has finished.
 *
 * @param[in] self      object that has to retrieve data
 * @param[in] pane      transponder edition pane
 * @param[in] transponderData    data of new/edited transponder
 **/
typedef void (*QBTransponderEditCallback)(void *self_,
                                          QBTransponderEditPane pane,
                                          QBBasicTransponderData transponderData);

/**
  * Create transponder edition pane
  *
  * @param[in] ctxMenu      context menu in which pane has to be created
  * @param[in] app          application hande
  * @param[in] level        level inside a menu
  * @param[in] editedTransponder    transponder to be edited or NULL for new
  * @param[in] callback     callback
  * @param[in] callbackData additional data for callback
  * @param[out] errorOut     error(s) that happened during creation
  * @return                  resulting pane or NULL in case of error
  **/
QBTransponderEditPane
QBTransponderEditPaneCreate(QBContextMenu ctxMenu,
                            SvApplication app,
                            int level,
                            QBDVBSatelliteTransponder editedTransponder,
                            QBTransponderEditCallback callback,
                            void *callbackData,
                            SvErrorInfo *errorOut);

#endif /* QBTRANSPONDEREDITPANE_H_ */

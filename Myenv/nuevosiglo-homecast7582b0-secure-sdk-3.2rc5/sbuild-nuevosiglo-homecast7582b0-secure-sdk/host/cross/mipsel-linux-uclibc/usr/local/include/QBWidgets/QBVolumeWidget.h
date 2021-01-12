/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBVOLUME_WIDGET_H
#define QBVOLUME_WIDGET_H

/**
 * @file QBVolumeWidget.h Volume widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBVolumeWidget QBVolumeWidget: Volume bar/mute icon widget
 * @ingroup QBWidgets
 * @{
 *
 * Volume bar/mute icon widget
 **/

#include <CUIT/Core/types.h>
#include <QBApplicationController.h>
#include <QBInput/QBInputEvent.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * Volume widget.
 * @class QBVolumeWidget
 * @extends QBGlobalWindow
 * @implements QBObserver
 *
 * Volume widget represents volume bar and mute icon shown in GUI
 **/
typedef struct QBVolumeWidget_ *QBVolumeWidget;

/**
 * Create volume widget.
 *
 * @param[in] app               application
 * @param[out] errorOut         error info
 * @return                      created volume widget instance, @c NULL if failed
 **/
QBVolumeWidget QBVolumeWidgetCreate(SvApplication app, SvErrorInfo *errorOut);

/**
 * Volume widget input event handler.
 *
 * @param[in] self    volume widget handle
 * @param[in] e       input event handle
 * @return            @c true if handled, @c false otherwise
 **/
bool QBVolumeWidgetInputEventHandler(QBVolumeWidget self, const QBInputEvent *e);

/**
 * Disable displaying volume widget GUI.
 *
 * @param[in] self    volume widget handle
 **/
void QBVolumeWidgetHideGUI(QBVolumeWidget self);

/**
 * Enable displaying volume widget GUI.
 *
 * @param[in] self    volume widget handle
 **/
void QBVolumeWidgetEnableGUI(QBVolumeWidget self);

/**
 * @}
 **/

#endif /* QBVOLUME_WIDGET_H */

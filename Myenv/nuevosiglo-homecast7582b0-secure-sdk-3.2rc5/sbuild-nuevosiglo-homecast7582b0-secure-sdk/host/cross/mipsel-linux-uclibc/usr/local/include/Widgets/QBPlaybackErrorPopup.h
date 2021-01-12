/*****************************************************************************
** Cubiware K.K. Software License Version 1.1
**
** Copyright (C) 2015 Cubiware K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware K.K.
**
** Any User wishing to make use of this Software must contact Cubiware K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_PLAYBACK_ERROR_POPUP_H_
#define QB_PLAYBACK_ERROR_POPUP_H_

/**
 * @file QBPlaybackErrorPopup.h Create playback error popup widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBApplicationController.h>
#include <QBWindowContext.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup ErrorPopup Error Popup
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Create playback error popup widget.
 *
 * @param[in] controller    application controller handle
 * @param[in] windowCtx     window context handle
 * @param[in] title         title C string
 * @param[in] explanation   explanation C string
 * @return                  playback error popup widget handle
 */
SvWidget QBPlaybackErrorPopupCreate(QBApplicationController controller, QBWindowContext windowCtx, const char* title, const char* explanation);

/**
 * @}
 **/

#endif

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBTVCHANNELFRAME_H_
#define QBTVCHANNELFRAME_H_

/**
 * @file QBTVChannelFrame.h QBTVChannelFrame widget API
 **/

#include <QBInput/QBInputEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <Services/core/QBTextRenderer.h>
#include <Services/core/playlistManager.h>
#include <CUIT/Core/types.h>
#include <main_decl.h>
#include <Logic/TVLogic.h>

/**
 * @defgroup QBTVChannelFrame TV Channel frame widget
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Callback inform that channel has changed.
 **/
typedef void (*QBTVChannelFrameCallback)(void *ptr, SvTVChannel channel);

/**
 * Callback inform that channel label has been rendered.
 **/
typedef void (*QBTVChannelFrameNameCallback)(void *ptr);

void QBTVChannelFrameSetChannel(SvWidget w, SvTVChannel channel);

/**
 * Create new QBTVChannelFrame widget
 *
 * @param[in] app          CUIT application handle
 * @param[in] name         widget name
 * @param[in] textRenderer text renderer
 * @param[in] playlists    playlist manager
 * @param[in] tvLogic      handle to TV logic
 * @return                 new QBTVChannelFrame widget or @c NULL in case of error
 **/
SvWidget
QBTVChannelFrameNew(SvApplication app,
                    const char *name,
                    QBTextRenderer textRenderer,
                    QBPlaylistManager playlists,
                    QBTVLogic tvLogic);

/**
 * Set pointer to callback function.
 *
 * @param w             SvWidget handle
 * @param callback      function pointer
 * @param callbackData  notifier handle
 **/
void QBTVChannelFrameNameChangedSetCallback(SvWidget w, QBTVChannelFrameNameCallback callback, void *callbackData);
void QBTVChannelFrameSetCallback(SvWidget w, QBTVChannelFrameCallback callback, void *callbackData);

void QBTVChannelFrameSetChannelNumberString(SvWidget w, const char *numStr);
void QBTVChannelFrameSetChannelName(SvWidget w, SvString name);
void QBTVChannelFrameSetChannelNumber(SvWidget w, int num);
int QBTVChannelFrameInputEventHandler(SvWidget w, const QBInputEvent *ie);

/**
 * @}
 **/

#endif /* QBTVCHANNELFRAME_H_ */

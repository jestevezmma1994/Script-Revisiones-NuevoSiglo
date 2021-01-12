/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef NEW_TV_OSD_H
#define NEW_TV_OSD_H

/**
 * @file NewTVOSD.h Main CubiTV OSD component from TV context
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Logic/EventsLogic.h>
#include <Logic/TVLogic.h>
#include <Services/QBAdService.h>
#include <Services/core/QBTextRenderer.h>
#include <Services/core/playlistManager.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <QBWindowContext.h>
#include <QBViewport.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup NewTVOSD Main CubiTV OSD component from TV context
 * @ingroup QBOSDMain
 * @{
 **/

/**
 * Main CubiTV OSD component from TV context.
 *
 * @class QBNewTVOSD NewTVOSD.h <Widgets/NewTVOSD.h>
 * @extends QBOSDMain
 * @implements TVOSD
 * @implements QBObserver
 **/
typedef struct QBNewTVOSD_ *QBNewTVOSD;

/**
 * @relates QBNewTVOSD
 *
 * QBNewTVOSD parameters.
 **/
typedef struct {
    SvApplication app;                  ///< CUIT application handle
    const char *widgetName;             ///< name of the widget
    QBApplicationController controller; ///< application cotroller
    QBAdService adService;              ///< ad service handler
    QBWindowContext tvContext;          ///< TV context handle
    QBTextRenderer textRenderer;        ///< text renderer
    QBPlaylistManager playlists;        ///< playlist manager
    QBViewport viewport;                ///< viewport
    EventsLogic eventsLogic;            ///< events logic
    QBTVLogic tvLogic;                  ///< tv logic
    QBParentalControl pc;               ///< parental controll
} QBNewTVOSDParams;

/**
 * Create TV OSD component.
 *
 * @public @memberof QBNewTVOSD
 *
 * @param[in] params QBNewTVOSD parameters
 * @return           new TV OSD component or @c NULL in case of error
 **/
QBNewTVOSD
QBNewTVOSDCreate(const QBNewTVOSDParams *params);

/**
 * @}
 **/

#endif

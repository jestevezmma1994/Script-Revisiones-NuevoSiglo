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

#ifndef IMAGEBROWSER_H
#define IMAGEBROWSER_H

#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <QBResourceManager/SvRBObject.h>
#include <QBWindowContext.h>
#include <main_decl.h>
#include <stdbool.h>

/**
 * @file imagebrowser.h Image browser window API
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 *
 * @defgroup CubiTV_windows_imageBrowser Image browser window
 * @ingroup CubiTV_windows
 * @{
 **/

typedef struct QBImageBrowserContext_t* QBImageBrowserContext;
typedef SvRID (*QBImageBrowserGetRIDFromPath)(SvGenericObject self, SvGenericObject nodePath);

/**
 * Image browser controller interface.
 **/
typedef const struct QBImageBrowserController_t {
    /**
     * Get RID of the bitmap for given path.
     *
     * @param[in] self_     handle to object implementing image browser controller
     * @param[in] nodePath  handle to requested path
     * @return              RID if node is an image, SV_RID_INVALID otherwise
     **/
    QBImageBrowserGetRIDFromPath getRIDFromPath;
} *QBImageBrowserController;

/**
 * Get instance of @ref SvInterface related to @ref QBImageBrowserController.
 * @return Instance of @ref SvInterface.
 **/
SvInterface QBImageBrowserController_getInterface(void);

/**
 * Create new image browser window.
 *
 * @param[in] appGlobals          AppGlobals handle
 * @param[in] parentnodePath_     Handle of node path to parent menu entry
 * @param[in] nodePosition        Position of the node in submenu
 * @param[in] controller          Handle of QBImageBrowserController instance
 * @param[in] slideshow           Controls slideshow mode
 * @param[in] slideshowDuration   Duration time in slideshow mode. Ignored when slideshow is false.
 *                                Negative value indicates default duration
 * @return                        Instance of image browser context
 **/
QBWindowContext QBImageBrowserContextCreate(AppGlobals appGlobals, SvGenericObject parentnodePath_,
                                            int nodePosition, SvGenericObject controller, bool slideshow, double slideshowDuration);

/**
 * Set m3u playlist to be played in background during slideshow.
 *
 * @param[in] self_  Handle of image browser
 * @param[in] URI    Handle of playlist URI
 */
void QBImageBrowserContextSetBackgroundMusicM3uPlaylist(QBWindowContext self_, SvString URI);

/**
 * Set playlist to be played in background during slideshow.
 *
 * @param[in] self_    Handle of image browser
 * @param[in] playlist Handle of the playlist. Playlist must contain SvContent objects.
 */
void QBImageBrowserContextSetBackgroundMusicPlaylist(QBWindowContext self_, SvArray playlist);

/**
 * @defgroup CubiTV_windows_imageBrowser_controllers Controllers used by image browser window
 **/

/**
 * @}
 **/

#endif /* IMAGEBROWSER_H */

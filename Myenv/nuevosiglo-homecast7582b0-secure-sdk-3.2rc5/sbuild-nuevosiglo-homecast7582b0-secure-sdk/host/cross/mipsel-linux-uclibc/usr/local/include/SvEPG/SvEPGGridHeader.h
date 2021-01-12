/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_EPG_GRID_HEADER_H_
#define SV_EPG_GRID_HEADER_H_

#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPG/SvEPGTextRenderer.h>
#include <SvEPG/SvEPGOverlay.h>
#include <SvEPG/SvEPGURLFormatter.h>

/**
 * @file SvEPGGridHeader.h EPG Grid Header Widget API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGGridHeader EPG grid header widget
 * @ingroup SvEPG
 * @{
 **/


/**
 * Create new EPG grid header widget.
 *
 * This method creates grid header widget. It will display some
 * information about TV channel passed by SvEPGGridHeaderSetChannel() method.
 *
 * Appearance of this widget is controlled by the Settings Manager.
 * It will use following parameters:
 *
 *  - width, height : integer, required,
 *  - horizPadding : integer, optional (default is 0),
 *  - IDFont, nameFont : font, required,
 *  - fontSize : integer, required,
 *  - textColor : color, required,
 *  - bgColor, activeBgColor : color, optional (default: transparent),
 *  - emptyLogo : bitmap, required,
 *  - logoWidth, logoHeight : integer, required,
 *  - IDWidth : integer, optional (default: auto),
 *  - itemSpacing : integer, optional (default: 0),
 *  - itemOrder : string, optional (default: "logo,ID,name").
 *
 * @param[in] app          CUIT application context
 * @param[in] widgetName   widget name
 * @param[in] renderer     EPG text renderer handle
 * @param[out] errorOut    error info
 * @return                 SvEPGGridHeader widget, NULL in case of error
 **/
extern SvWidget
SvEPGGridHeaderNew(SvApplication app,
                   const char *widgetName,
                   SvEPGTextRenderer renderer,
                   SvErrorInfo *errorOut);

/**
 * Install callback for retrieving URL of channel logo
 * from TV channel object.
 *
 * @param[in] header       EPG grid header widget
 * @param[in] formatter    callback function
 * @param[out] errorOut    error info
 **/
extern void
SvEPGGridHeaderSetURLFormatter(SvWidget header,
                               SvEPGURLFormatter formatter,
                               SvErrorInfo *errorOut);

/**
 * Install or uninstall EPG overlay.
 *
 * @param[in] grid         EPG grid header widget
 * @param[in] overlay      EPG overlay: an object implementing
 *                         SvEPGOverlay interface (NULL to remove
 *                         previously installed one)
 **/
extern void
SvEPGGridHeaderSetOverlay(SvWidget grid,
                          SvObject overlay);

/**
 * Set TV channel to be shown.
 *
 * @param[in] header       EPG grid header widget
 * @param[in] channel      TV channel to be shown
 **/
extern void
SvEPGGridHeaderSetChannel(SvWidget header,
                          SvTVChannel channel);

/**
 * Update meta data of a TV channel.
 *
 * @param[in] header       EPG grid header widget
 * @param[in] channel      TV channel after update
 **/
extern void
SvEPGGridHeaderUpdate(SvWidget header,
                      SvTVChannel channel);

/**
 * Mark EPG grid header widget as active/inactive.
 *
 * @param[in] header       EPG grid header widget
 * @param[in] active       @c true to set active
 **/
extern void
SvEPGGridHeaderSetActive(SvWidget header,
                         bool active);


/**
 * @}
 **/

#endif

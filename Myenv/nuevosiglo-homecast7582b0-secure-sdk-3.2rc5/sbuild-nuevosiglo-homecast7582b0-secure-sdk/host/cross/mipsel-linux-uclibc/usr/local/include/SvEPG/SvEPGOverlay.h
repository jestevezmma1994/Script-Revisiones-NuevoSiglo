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

#ifndef SV_EPG_OVERLAY_H_
#define SV_EPG_OVERLAY_H_

/**
 * @file SvEPGOverlay.h EPG Overlay Interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvObject.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <CUIT/Core/widget.h>


/**
 * @defgroup SvEPGOverlay EPG overlay interface
 * @ingroup SvEPG
 * @{
 **/

/**
 * SvEPGOverlay interface.
 **/
typedef const struct SvEPGOverlay_t {
    /**
     * Setup overlay for an EPG grid header widget.
     *
     * This method is called to create overlay widget for SvEPGGridHeader,
     * one of the elements of an EPG grid. Created overlay widget will be
     * attached at the z-level 2 (above the default contents of the header
     * widget, which is attached at the z-level 1). If you want to place
     * the overlay below default contents, attach it yourself before
     * returning.
     *
     * Overlay widget will be destroyed when SvEPGGridHeader is destroyed.
     * If its contents changes, this method will be
     * called again to update or destroy old overlay and create new overlay.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGOverlay
     * @param[in] header    EPG grid header widget
     * @param[in] channel   TV channel handle
     * @param[in] prev      existing overlay widget (@c NULL if not present)
     * @return              created overlay widget or @c NULL if not needed
     **/
    SvWidget (*setupHeader)(SvObject self_,
                            SvWidget header,
                            SvTVChannel channel,
                            SvWidget prev);

    /**
     * Setup overlay for an EPG event widget.
     *
     * This method is called to create overlay widget for every created
     * EPG event widget. Created overlay widget will be attached at the
     * z-level 2 (above the default contents of the header widget, which
     * is attached at the z-level 1). If you want to place the overlay
     * below default contents, attach it yourself before returning.
     *
     * Overlay widget will be destroyed when EPG event widget is destroyed
     * or its contents changes. In the latter case, this method will be
     * called again to create updated overlay.
     *
     * @param[in] self_         handle to an object implementing @ref SvEPGOverlay
     * @param[in] widget        EPG event widget
     * @param[in] channel       TV channel handle
     * @param[in] event         EPG event handle
     * @param[in] spacing       spacing info for the icons
     * @param[in] visibleWidth  width of the visible part of the overlay
     * @return                  created overlay widget or @c NULL if not needed
     **/
    SvWidget (*setupEvent)(SvObject self_,
                           SvWidget widget,
                           SvTVChannel channel,
                           SvEPGEvent event,
                           int spacing,
                           int visibleWidth);
} *SvEPGOverlay;

/**
 * Get runtime type identification object representing
 * SvEPGOverlay interface.
 *
 * @return SvEPGOverlay interface object
 **/
extern SvInterface
SvEPGOverlay_getInterface(void);

/**
 * @}
 **/


#endif

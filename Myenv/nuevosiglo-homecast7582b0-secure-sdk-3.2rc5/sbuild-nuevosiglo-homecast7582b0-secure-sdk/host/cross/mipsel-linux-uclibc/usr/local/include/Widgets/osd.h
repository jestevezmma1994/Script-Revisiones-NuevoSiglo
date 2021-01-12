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
#ifndef OSD_H_
#define OSD_H_

#include <stdbool.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/QBUIEvent.h>


/**
 * @file osd.h CubiTV OSD background widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup CubiTVOSD CubiTV OSD background widget
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * UI event sent by the OSD when its state changes.
 * @class SvOSDStateChangedEvent
 * @extends QBUIEvent
 **/
typedef struct SvOSDStateChangedEvent_ *SvOSDStateChangedEvent;

/**
 * SvOSDStateChangedEvent class internals.
 **/
struct SvOSDStateChangedEvent_ {
    /// super class
    struct QBUIEvent_ super_;
    /// @c true if OSD has become visible, @c false if OSD is hidden
    bool OSDIsVisible;
};

/**
 * Get runtime type identification object representing OSD state changed event class.
 *
 * @relates SvOSDStateChangedEvent
 *
 * @return OSD state changed event class
 **/
extern SvType
SvOSDStateChangedEvent_getType(void);


/**
 * Prevent OSD from appearing/disappearing.
 *
 * @param[in] osd         OSD Widget.
 */
extern void svOSDLock(SvWidget osd);

/**
 * Revert the effect of svOSDLock.
 *
 * @param[in] osd         OSD widget
 */
extern void svOSDUnlock(SvWidget osd);

/**
 * Check if OSD is visible.
 *
 * @param[in] osd         OSD widget.
 * @returns               @c True if widget is visible @c false otherwise.
 */
extern bool svOSDIsVisible(SvWidget osd);

/**  Make OSD remain on screen for widget specific duration.
 *
 * @param[in] osd          Widget that will be affected by function.
 */
extern void svOSDTouch(SvWidget osd);

/**
 * Make OSD appear on the screen.
 *
 * @param[in] osd          Widget that will be affected by function.
 * @param[in] immediate    If @c true widget appears instantly, otherwise it
 *                         slides into display with a move effect.
 */
extern void svOSDShow(SvWidget osd, bool immediate);

/**
 * Make OSD disappear.
 *
 * @param[in] osd          Widget that will be affected by function.
 * @param[in] immediate    If @c true widget disappears instantly, otherwise it
 *                         slides out of display with a move effect.
 */
extern void svOSDHide(SvWidget osd, bool immediate);

/**
 * Attach OSD to parent.
 *
 * @param[in] osd           OSD widget handle
 * @param[in] parent        parent widget to attach to
 * @param[in] level         zorder on parent at which osd should be attached
 */
void svOSDAttachToParent(SvWidget osd, SvWidget parent, int level);

/**
 * Create OSD widget.
 *
 * This widget is just a rectangle bitmap, with functionality
 * of appearing invoked by a function call and disappearing after specified period
 * of time or through a function call. It is created using Settings Manager.
 * Parameters:
 *
 * - width, height - int, required if bg not specified
 * - bg - bitmap, required if at least one of: width, height, bgColor not specified,
 * - bgColor - color, required if bg not specified,
 * - duration - double, optional, specifies duration of hide/show effect,
 * - hideDelay - double, optional, specifies delay when widget starts to disappear,
 * - mode - string, optional, top, bottom or fixed, specifies widget placement on screen.
 * - noHideHeigth - int, optional, used for top and bottom modes, specifies height that stays on the screen.
 *
 * @param app              CUIT application handle.
 * @param widgetName       Widget's name in settings file.
 * @returns                Newly created widget or NULL.
 */
extern SvWidget svOSDNew(SvApplication app, const char *widgetName);

/**
 * @}
 **/

#endif

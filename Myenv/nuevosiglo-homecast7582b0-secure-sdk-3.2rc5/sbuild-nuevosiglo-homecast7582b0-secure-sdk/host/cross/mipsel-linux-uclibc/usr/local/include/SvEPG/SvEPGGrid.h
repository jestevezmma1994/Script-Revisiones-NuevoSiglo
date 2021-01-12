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

#ifndef SV_EPG_GRID_H_
#define SV_EPG_GRID_H_

#include <SvPlayerKit/SvEPGEvent.h>
#include <CUIT/Core/QBUIEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGDataWindow.h>
#include <SvEPG/SvEPGTextRenderer.h>
#include <SvEPG/SvEPGOverlay.h>
#include <SvEPG/SvEPGURLFormatter.h>

/**
 * @file SvEPGGrid.h EPG Grid Widget API.
 **/

/**
 * @defgroup SvEPGGrid EPG grid widget
 * @ingroup SvEPG
 * @{
 **/

/**
 * UI event sent by EPG grid when focus is moved between events.
 * @class SvEPGGridFocusChangedEvent
 * @extends QBUIEvent
 **/
typedef struct SvEPGGridFocusChangedEvent_ *SvEPGGridFocusChangedEvent;

/**
 * SvEPGGridFocusChangedEvent class internals.
 **/
struct SvEPGGridFocusChangedEvent_ {
    /// super class
    struct QBUIEvent_ super_;
    /// a handle to the active channel, @c NULL if none
    SvTVChannel activeChannel;
    /// a handle to the active EPG event, @c NULL if none
    SvEPGEvent activeEvent;
};

/**
 * Get runtime type identification object representing EPG grid focus changed event class.
 *
 * @relates SvEPGGridFocusChangedEvent
 *
 * @return EPG grid focus changed event class
 **/
extern SvType
SvEPGGridFocusChangedEvent_getType(void);


/**
 * Create new EPG grid widget.
 *
 * This method creates grid widget. It will display main part of
 * the EPG: a grid of TV events, with TV channels on vertical axis
 * and time on horizontal axis.
 *
 * Appearance of this widget is controlled by the Settings Manager.
 * It will use following parameters:
 *
 *  - width, height : integer, required,
 *  - timeAxisSpacing, stripeSpacing, headersSpacing : integer, optional (default is 0),
 *  - marginHeight : integer, optional (default is 2),
 *  - changeDuration : double, optional (default is 0.0),
 *  - timeLineColor : color, optional (default is 00000000),
 *  - timeLineWidth : integer, optional (default is 0),
 *  - timeScale : integer, optional (default is @a timeScale parameter).
 *
 * @param[in] app          CUIT application context
 * @param[in] widgetName   widget name
 * @param[in] dataWindow   EPG data window handle
 * @param[in] renderer     EPG text renderer handle
 * @param[in] timeScale    number of pixels per one hour
 *                         (pass @c 0 to use value from settings)
 * @param[in] displayer    EPG displayer handle
 * @param[out] errorOut    error info
 * @return                 SvEPGGrid widget, NULL in case of error
 **/
extern SvWidget
SvEPGGridNew(SvApplication app,
             const char *widgetName,
             SvEPGDataWindow dataWindow,
             SvEPGTextRenderer renderer,
             unsigned int timeScale,
             SvObject displayer,
             SvErrorInfo *errorOut);

/**
 * Get currently focused channel.
 *
 * @param[in] grid         SvEPGGrid widget
 * @return                 currently focused channel, NULL if unavailable
 **/
extern SvTVChannel
SvEPGGridGetActiveChannel(SvWidget grid);

/**
 * Change active (focused) channel.
 *
 * @param[in] grid      SvEPGGrid widget
 * @param[in] channelID ID of a TV channel to activate
 * @return              channel with @a channelID, NULL if not found
 **/
extern SvTVChannel
SvEPGGridSetActiveChannel(SvWidget grid,
                          SvValue channelID);

/**
 * Set currently displayed time.
 *
 * @param[in] grid      SvEPGGrid widget
 * @param[in] timePoint point in time that should be displayed
 *                      in the center of the EPG grid
 * @param[out] errorOut error info
 **/
extern void
SvEPGGridSetActivePosition(SvWidget grid,
                           time_t timePoint,
                           SvErrorInfo *errorOut);

/**
 * Install callback for retrieving URL of channel logo
 * from TV channel object.
 *
 * @param[in] grid         SvEPGGrid widget
 * @param[in] formatter    callback function
 * @param[out] errorOut    error info
 **/
extern void
SvEPGGridSetURLFormatter(SvWidget grid,
                         SvEPGURLFormatter formatter,
                         SvErrorInfo *errorOut);

/**
 * Install or uninstall EPG overlay.
 *
 * @param[in] grid         SvEPGGrid widget
 * @param[in] overlay      EPG overlay: an object implementing
 *                         SvEPGOverlay interface (NULL to remove
 *                         previously installed one)
 * @param[out] errorOut    error info
 **/
extern void
SvEPGGridSetOverlay(SvWidget grid,
                    SvObject overlay,
                    SvErrorInfo *errorOut);

/**
 * Set limit on displayed time range.
 *
 * @param[in] grid         SvEPGGridWidget
 * @param[in] start        start of the displayed range
 * @param[in] end          end if the displayed range
 * @param[out] errorOut    error info
 **/
extern void
SvEPGGridSetTimeLimit(SvWidget grid,
                      int start, int end,
                      SvErrorInfo *errorOut);

/**
 * Change data window.
 *
 * @param[in] grid         SvEPGGrid widget
 * @param[in] dataWindow   EPG data window handle
 * @param[out] errorOut    error info
 **/
extern void
SvEPGGridSetDataWindow(SvWidget grid,
                       SvEPGDataWindow dataWindow,
                       SvErrorInfo *errorOut);

/**
 * Reinitialize data window.
 *
 * @param[in] grid          SvEPGGrid widget
 **/
extern void
SvEPGGridReinitialize(SvWidget grid);

typedef SvEPGEventDesc (*SvEPGDescFunc)(void* target, SvEPGEvent event);

extern void
SvEPGGridSetDescFunc(SvWidget grid, SvEPGDescFunc func, void *target);

extern void
SvEPGGridSetFormatId(SvWidget grid, const char *formatId);

extern time_t
SvEPGGridGetTimePoint(SvWidget grid);

/**
 * @}
 **/

#endif

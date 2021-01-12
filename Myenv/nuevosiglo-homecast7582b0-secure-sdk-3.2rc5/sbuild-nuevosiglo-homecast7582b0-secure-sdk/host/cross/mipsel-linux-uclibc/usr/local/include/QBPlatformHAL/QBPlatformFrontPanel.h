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

#ifndef QB_PLATFORM_FRONT_PANEL_H_
#define QB_PLATFORM_FRONT_PANEL_H_

/**
 * @file QBPlatformFrontPanel.h Front panel display control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <time.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformFrontPanel Front panel display control
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Get front panel display capabilities.
 *
 * @param[out] capabilities front panel display capabilities
 * @return             @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformGetFrontPanelCapabilities(QBFrontPanelCapabilities *capabilities);

/**
 * Get flags describing basic front panel display capabilities.
 *
 * This is a wrapper for QBPlatformGetFrontPanelCapabilities(),
 * that returns only QBFrontPanelCapabilities::capabilities value.
 *
 * @return      bitwise mask of QBFrontPanelCapability_* values
 **/
extern unsigned int
QBPlatformGetFrontPanelFlags(void);

/**
 * Show text on front panel.
 *
 * @param[in] text     text to display
 * @return             @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformShowOnFrontPanel(const char *text);

/**
 * Switch front panel display to clock mode.
 *
 * @param[in] enable    @c true to enable clock mode, @c false to disable
 * @param[in] now       current time (e.g. from SvTimeNow())
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformClockOnFrontPanel(bool enable, time_t now);

/**
 * Update clock on front panel display.
 *
 * @param[in] now       current time (e.g. from SvTimeNow())
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformUpdateClockOnFrontPanel(time_t now);

/**
 * Set format of the clock shown on the front panel display.
 *
 * @param[in] format    time format compatible with strftime()
 * @return              @c 0 on success, @c -1 in case of error
 */
extern int
QBPlatformClockOnFrontPanelSetFormat(const char *format);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

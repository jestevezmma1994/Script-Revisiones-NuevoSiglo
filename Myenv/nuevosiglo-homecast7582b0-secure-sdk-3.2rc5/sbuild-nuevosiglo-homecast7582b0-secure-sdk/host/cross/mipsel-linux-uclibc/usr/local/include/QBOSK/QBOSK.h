/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_OSK_WIDGET_H_
#define QB_OSK_WIDGET_H_

/**
 * @file QBOSK.h
 * @brief On Screen Keyboard widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>

#include <SvCore/SvErrorInfo.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include "QBOSKTypes.h"


/**
 * @defgroup QBOSKWidget On Screen Keyboard widget
 * @ingroup QBOSK
 * @{
 *
 * Widget implementing actual OSK.
 **/

/**
 * QBOSKParams parameters.
 * Struct used for creating QBOSK with given parameters.
 **/
typedef struct QBOSKParams_ {
    unsigned int width;         ///< OSK widget width
    unsigned int height;        ///< OSK widget height
    unsigned int padding;       ///< internal widget padding

    unsigned int keyWidth;      ///< key height
    unsigned int keyHeight;     ///< key width
    unsigned int horizMargin;   ///< horizontal margin
    unsigned int vertMargin;    ///< vertical margin

    unsigned int fontSize;      ///< font size
} QBOSKParams;

/**
 * Create new On Screen Keyboard widget from given parameters and settings manager.
 *
 * @param[in] app           CUIT application handle
 * @param[in] name          widget name
 * @param[in] params        osk parameters
 * @param[out] errorOut     error info
 * @return                  created widget, @c NULL in case of error
 **/
SvWidget
QBOSKNewFromParamsAndSM(SvApplication app,
                        const char *name,
                        QBOSKParams params,
                        SvErrorInfo *errorOut);

/**
 * Create new On Screen Keyboard widget.
 *
 * @param[in] app           CUIT application handle
 * @param[in] name          widget name
 * @param[out] errorOut     error info
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBOSKNew(SvApplication app,
         const char *name,
         SvErrorInfo *errorOut);

/**
 * Create new On Screen Keyboard widget from ratio settings.
 *
 * @param[in] app           CUIT application handle
 * @param[in] name          widget name
 * @param[in] parentWidth   parent width
 * @param[in] parentHeight  parent height
 * @param[out] errorOut     error info
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBOSKNewFromRatio(SvApplication app,
                  const char *name,
                  unsigned int parentWidth,
                  unsigned int parentHeight,
                  SvErrorInfo *errorOut);

/**
 * Setup keyboard layout.
 *
 * @param[in] OSK           OSK widget handle
 * @param[in] renderer      OSK renderer handle
 * @param[in] map           OSK map handle
 * @param[in] layoutID      identifier of the initial layout
 * @param[out] errorOut     error info
 **/
extern void
QBOSKSetKeyboardMap(SvWidget OSK,
                    QBOSKRenderer renderer,
                    QBOSKMap map,
                    SvString layoutID,
                    SvErrorInfo *errorOut);

/**
 * Force widget redraw.
 *
 * @param[in] OSK           OSK widget handle
 **/
extern void
QBOSKForceRender(SvWidget OSK);

/**
 * Change keyboard layout.
 *
 * @param[in] OSK           OSK widget handle
 * @param[in] layoutID      OSK layout ID
 * @param[out] errorOut     error info
 **/
extern void
QBOSKSelectLayout(SvWidget OSK,
                  SvString layoutID,
                  SvErrorInfo *errorOut);

/**
 * Get the current keyboard layout
 *
 * @param[in] OSK           OSK widget handle
 * @return                  OSK layout ID, @c NULL in case of error
 **/
extern SvString
QBOSKGetSelectedLayout(SvWidget OSK);

/**
 * Change OSK key active state.
 *
 * @param[in] OSK           OSK widget handle
 * @param[in] keyID         OSK key ID
 * @param[in] active        @c true to make key active, @c false for inactive
 * @param[out] errorOut     error info
 **/
extern void
QBOSKSetKeyActive(SvWidget OSK,
                  SvString keyID,
                  bool active,
                  SvErrorInfo *errorOut);

/**
 * Get ID of currently focused key.
 *
 * @param[in] OSK       OSK widget handle
 * @return              ID of the focused key
 **/
extern SvString
QBOSKGetFocusedKeyID(SvWidget OSK);

/**
 * Set focus to specific key.
 *
 * @param[in] OSK       OSK widget handle
 * @param[in] keyID     ID of the key
 **/
extern void
QBOSKSetFocusToKey(SvWidget OSK, SvString keyID);

/**
 * Use OSK widget to handle input event.
 *
 * @param[in] OSK    OSK widget handle
 * @param[in] e      event to be handled
 *
 * @return           flag indicating if the event was handled
 **/
extern bool
QBOSKHandleInputEvent(SvWidget OSK, SvInputEvent e);
/**
 * @}
 **/


#endif

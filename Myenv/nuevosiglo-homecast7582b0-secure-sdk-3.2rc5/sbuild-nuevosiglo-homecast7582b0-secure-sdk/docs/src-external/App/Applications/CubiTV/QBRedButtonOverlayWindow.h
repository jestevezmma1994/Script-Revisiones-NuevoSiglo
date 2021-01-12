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

#ifndef QBREDBUTTONOVERLAYWINDOW_H_
#define QBREDBUTTONOVERLAYWINDOW_H_

/**
 * @file QBRedButtonOverlayWindow.h
 * @brief Red Button Overlay Window
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBRedButtonOverlayWindow Red Button Overlay Window
 * @ingroup TV
 * @{
 **/

#include <main_decl.h>
#include <SvFoundation/SvHashTable.h>

/**
 * This class wraps @ref QBRedButtonOverlay in a fullscreen @ref QBLocalWindow.
 */
typedef struct QBRedButtonOverlayWindow_ *QBRedButtonOverlayWindow;

/**
 * Create QBRedButtonOverlayWindow.
 *
 * @param[in] appGlobals    appGlobals handle
 * @param[in] data          red button overlay widget content data, see @ref QBRedButtonOverlayNew
 * @return                  created QBRedButtonOverlayWindow
 */
QBRedButtonOverlayWindow QBRedButtonOverlayWindowNew(AppGlobals appGlobals, SvHashTable data);

/**
 * Replace content of QBRedButtonOverlayWindow.
 *
 * @param[in] self          QBRedButtonOverlayWindow handle
 * @param[in] data          red button overlay widget content data, see @ref QBRedButtonOverlayNew
 */
void QBRedButtonOverlayWindowReplaceContent(QBRedButtonOverlayWindow self, SvHashTable data);

/**
 * Show QBRedButtonOverlayWindow.
 *
 * @param[in] self          QBRedButtonOverlayWindow handle
 */
void QBRedButtonOverlayWindowShow(QBRedButtonOverlayWindow self);

/**
 * Hide QBRedButtonOverlayWindow.
 *
 * @param[in] self          QBRedButtonOverlayWindow handle
 */
void QBRedButtonOverlayWindowHide(QBRedButtonOverlayWindow self);

/**
 * Test if QBRedButtonOverlayWindow is shown.
 *
 * @param[in] self          QBRedButtonOverlayWindow handle
 * @return                  @c true if QBRedButtonOverlayWindow is shown
 */
bool QBRedButtonOverlayWindowIsVisible(QBRedButtonOverlayWindow self);

/**
 * @}
 **/

#endif /* QBREDBUTTONOVERLAYWINDOW_H_ */

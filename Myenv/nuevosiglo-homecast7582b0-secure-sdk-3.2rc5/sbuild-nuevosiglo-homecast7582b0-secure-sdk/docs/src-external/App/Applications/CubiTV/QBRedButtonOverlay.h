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

#ifndef QBREDBUTTONOVERLAY_H_
#define QBREDBUTTONOVERLAY_H_

/**
 * @file QBRedButtonOverlay.h
 * @brief Red Button Overlay Widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBRedButtonOverlay Red Button Overlay Widget
 * @ingroup CubiTV_widgets
 * @{
 **/

#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>

/**
 * Create RedButtonOverlay widget.
 *
 * RedButtonOverlay displays one bitmap and labels (titles). When user presses
 * a color button, then associated bitmap is shown and a label is highlighted.
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    widget name
 * @param[in] data          (details below)
 *
 * Parameter data is a hash table with the following structure:
 *
 *     {
 *         "images": {
 *             "red": SvRBBitmap,
 *             "green": SvRBBitmap,
 *             "yellow": SvRBBitmap,
 *             "blue": SvRBBitmap
 *         },
 *         "titles": {
 *             "red": SvString,
 *             "green": SvString,
 *             "yellow": SvString,
 *             "blue": SvString
 *         }
 *     }
 *
 * @return                  created widget, or @c NULL in case of error
 */
SvWidget QBRedButtonOverlayNew(SvApplication app, const char *widgetName, SvHashTable data);

/**
 * Replace content with new data.
 *
 * @param[in] self      RedButtonOverlay widget handle
 * @param[in] data      see @ref QBRedButtonOverlayNew
 */
void QBRedButtonOverlayReplaceContent(SvWidget self, SvHashTable data);

/**
 * Switch back to the overlay that should be displayed by default.
 *
 * @param[in] self      RedButtonOverlay widget handle
 */
void QBRedButtonOverlayReset(SvWidget self);

/**
 * @}
 **/

#endif /* QBREDBUTTONOVERLAY_H_ */

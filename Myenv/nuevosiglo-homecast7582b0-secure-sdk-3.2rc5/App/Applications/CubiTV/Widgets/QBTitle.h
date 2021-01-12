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

#ifndef QBTITLE_H_
#define QBTITLE_H_

#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>

typedef void (*QBTitleCallback)(void *ptr, SvWidget title);

SvWidget
QBTitleNew(SvApplication app, char const *widgetName, QBTextRenderer renderer);

/**
 * Set the text of the given part of the title.
 *
 * QBTitleSetText sets a new text string, which will be displayed at the given
 * position in the title.  Note that changing the title at position pos, will
 * cause all elements following pos to disappear for a moment.
 *
 * It must be noted that the title bar displays the parts of the title in order,
 * separating them by > marks until an empty part is encountered or all parts
 * are displayed.  If the part at position pos is empty, elements at positions
 * pos + 1, pos + 2, etc are not displayed.  When a given part of the title is
 * not visible, it is still possible to change its text and the widget will
 * remember its value.
 *
 * @param[in] w      the QBTitle object
 * @param[in] text   new title text
 * @param[in] pos    the index of the part, which will be changed
 */
void
QBTitleSetText(SvWidget w, SvString text, int pos);
void
QBTitleSetCallback(SvWidget w, QBTitleCallback callback, void *callbackData);

#endif /* QBTITLE_H_ */

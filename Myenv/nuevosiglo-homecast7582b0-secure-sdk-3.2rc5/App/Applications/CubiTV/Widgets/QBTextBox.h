/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TEXT_BOX_H_
#define QB_TEXT_BOX_H_

#include <CUIT/Core/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * @file QBTextBox.h
 * @brief Widget with capability to display long text message.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBTextBox Widget with capability to display long text
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Create textBox widget using settings from the Settings Manager.
 *
 * This method creates textBox widget that can display very long text in scrollable window.
 * If you want to customize QBTextBox through settings, you need to look into QBScrollView
 * documentation (class that is used internally by QBTextBox).
 *
 * To properly set width for widget through settings, you need to set following
 * parameters to the same value:
 *  - width : integer,
 *  - itemWidth : integer,
 *  - viewport.width : integer,
 *  - text.width : integer
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    widget name
 * @param[out] errorOut     error output
 * @return                  newly created widget or NULL in case of error
 **/
extern SvWidget
QBTextBoxCreate(SvApplication app, const char *widgetName, SvErrorInfo *errorOut);

/**
 * Add text to widget text field. TextBox for performance reasons requires that newline
 * mark appears from time to time. If it won't show up, newline will be automatically
 * added to break the text.
 *
 * @param[in] w         widget handle
 * @param[in] text      text to add
 */
extern void
QBTextBoxAddText(SvWidget w, SvString text);

/**
 * Add text to widget text field. TextBox for performance reasons requires that newline
 * mark appears from time to time. If it won't show up in \p paragraphLimit chars, newline
 * will be automatically added to break the text.
 *
 * @param[in] w                 widget handle
 * @param[in] text              text to add
 * @param[in] paragraphLimit    specifies maximum number of chars allowed in paragraph
 */
extern void
QBTextBoxAddTextWithLimit(SvWidget w, SvString text, int paragraphLimit);


/**
 * Sets widget height.
 *
 * @param[in] w         widget handle
 * @param[in] height    new height
 */
extern void
QBTextBoxSetHeight(SvWidget w, int height);

/**
 * @}
 **/


#endif /* QB_TEXT_BOX_H_ */

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


#ifndef QBMENUCAPTION_H_
#define QBMENUCAPTION_H_

/**
 * @file QBMenuCaption.h
 * @brief QBMenuCaption widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>
#include <main_decl.h>

/**
 * @defgroup QBMenuCaption QBMenuCaption widget class
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Create QBMenuCaption widget
 *
 * @param[in] app handle to a SvApplication instance
 * @param[in] widgetName widget name to be matched with settings
 * @param[in] renderer handle to a QBTextRenderer instance
 *
 * @return    QBMenuCaption widget
 **/
SvWidget QBMenuCaptionNew(SvApplication app, char const *widgetName, QBTextRenderer renderer);

/**
 * Register a QBMenuCaption widget to be used in the main menu tree
 *
 * @param[in] caption    QBMenu caption widget handle
 * @param[in] appGlobals appGlobals handle
 *
 **/
void QBMenuCaptionRegister(SvWidget caption, AppGlobals appGlobals);

/**
 * Get width of the QBMenuCaption widget's label
 *
 * @param[in] w QBMenu caption widget handle
 *
 * @return      QBMenuCaption widget's label width
 **/
unsigned int QBMenuCaptionGetLabelWidth(SvWidget w);

/**
 * Get width of the QBMenuCaption widget's icon
 *
 * @param[in] w QBMenu caption widget handle
 *
 * @return      QBMenuCaption widget's icon width
 **/
unsigned int QBMenuCaptionGetIconWidth(SvWidget w);

/**
 * Get overall width of the QBMenuCaption widget
 *
 * @param[in] w QBMenu caption widget handle
 *
 * @return      Overall QBMenuCaption widget's width
 **/
unsigned int QBMenuCaptionGetWidth(SvWidget w);

/**
 * Add a listener to be notified about every change of the QBMenuCaption widget
 *
 * @param[in] caption    QBMenu caption widget handle
 * @param[in] listener   an object to be regiestered as a listener
 *
 **/
void QBMenuCaptionAddListener(SvWidget caption, SvGenericObject listener);

/**
 * Remove a listener that is notified about every change of the QBMenuCaption widget
 *
 * @param[in] caption    QBMenu caption widget handle
 * @param[in] listener   an object to be unregiestered as a listener
 *
 **/
void QBMenuCaptionRemoveListener(SvWidget caption, SvGenericObject listener);

/**
 * QBMenuCaptionListener interface.
 **/
typedef const struct QBMenuCaptionListener_ {
    /**
     * Notify that a shape of the QBMenuCaption widget has been changed
     *
     * @param[in] self_     handle to an object implementing @ref QBMenuCaptionListener
     *
     **/
    void (*changed) (SvGenericObject self_);
} *QBMenuCaptionListener;

/**
 * Get runtime type identification of an object representing
 * @return QBMenuCaption listener interface.
 **/
SvInterface QBMenuCaptionListener_getInterface(void);

/**
 * @}
 **/

#endif /* QBMENUCAPTION_H_ */

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

#ifndef QB_XHTML_TXT_WIDGET_H
#define QB_XHTML_TXT_WIDGET_H

/**
 * Widget class that will paint the widgets in console.
 * This is an example widget class good for testing the XHTML layouts and performing some unit tests.
 * The only public function of @c QBTxtWidget is painting, as their creation is handled by factories.
 *
 * @class QBTxtWidget
 * @extends SvObject
 */
typedef struct QBTxtWidget_ *QBTxtWidget;

/**
 * Paint method for a txt console widget.
 * Thi smethod will cause the widget to be painted on the canvas. You can later print the canvas to
 * the screen, or file using some logging function.
 *
 * @param [in] self             widget reference
 * @param [in] canvas           buffer to print to. Buffer size must be canvasWidth * canvasHeight.
 * @param [in] canvasWidth      width of canvas that you want to print to. Keep in mind that the canves should be at least
 *                              3 pixels wider than the widget to fit in frame around canvas and '\n' at end of line
 * @param [in] canvasHeight     height of canvas that you want to print to. Keep in mind that the canves should be at least
 *                              2 pixels higher than the widget to fit in frame around canvas
 */
extern void
QBTxtWidgetPaint(QBTxtWidget self, char *canvas, int canvasWidth, int canvasHeight);

/**
 * Factory for creating Txt widget containers (widget for other widgets).
 * The factory implements @c QBXHTMLContainerFactory so it can be passed as a parameter to @c QBXHTMLAssemble.
 * The factory creates containers on demand, and assembles widgets together when needed.
 *
 * @class QBXHTMLTxtContainerFactory
 * @extends SvObject
 * @implements QBXHTMLContainerFactory
 **/

/**
 * Factory for creating Txt widgets.
 * The factory implements @c QBXHTMLWidgetFactory and @c QBNodeVisitor so it can be passed as a parameter to @c QBXHTMLAssemble.
 * During visitation of some nodes (img, input, select etc) factory creates widgets and inserts them into the results hashtable
 * which is later returned as a result of visitation.
 *
 * @class QBXHTMLTxtContainerFactory
 * @extends SvObject
 * @implements QBNodeVisitor
 * @implements QBXHTMLWidgetFactory
 **/
typedef struct QBXHTMLTxtWidgetFactoryVisitor_ *QBXHTMLTxtWidgetFactoryVisitor;

/**
 * Create new instance of QBXHTMLTxtWidgetFactoryVisitor.
 *
 * @return      new instance of QBXHTMLTxtWidgetFactoryVisitor or @c NULL in case of error
 **/
extern QBXHTMLTxtWidgetFactoryVisitor
QBXHTMLTxtWidgetFactoryVisitorCreate(void);

#endif //QB_XHTML_TXT_WIDGET_H

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

#ifndef QB_XHTML_QUICK_VISITOR_H
#define QB_XHTML_QUICK_VISITOR_H

/**
 * @file QBXHTMLQuickVisitor.h XHTML tree quick visitor
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBXHTMLWidgets/QBXHTMLVisitor.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * @defgroup QBXHTMLQuickVisitor XHTML tree quick visitor
 * @ingroup GUI
 * @{
 *
 * The quick visitor class. This class can be used to quickly create a visitor.
 **/

/**
 * General purpose visitor class which can be used for visiting without declaring a separate visitor class.
 * Object of this class can be created instead of creating a separate visitor class. After creation you
 * can set handlers for types of nodes you are interested in visiting, and later visit some nodes with
 * this visitor.
 *
 * @class QBXHTMLQuickVisitor
 * @extends SvObject
 * @implements QBNodeVisitor
 **/
typedef struct QBXHTMLQuickVisitor_ *QBXHTMLQuickVisitor;

/**
 * Create new instance of QBXHTMLQuickVisitor.
 *
 * @param [in] privateData  pointer to this data will be stored in the visitor, and can later be obtained when visiting a node
 * @param [in] superVisit   if this patameter is @c true then when no callback for particular type of node is registered in quick visitor
 *                          the node will be visited as its superclass (@c QBXHTMLVisitVisitableNode). If this parameter is @c false
 *                          no upcasting will be used.
 * @param [out] errorOut    error output
 * @return                  new instance of QBXHTMLQuickVisitor or @c NULL in case of error
 **/
extern QBXHTMLQuickVisitor
QBXHTMLQuickVisitorCreate(void *privateData, bool superVisit, SvErrorInfo *errorOut);

/**
 * Get private data passed to visitor upon construction (@c QBXHTMLQuickVisitorCreate).
 *
 * @param [in] self     quick visitor reference
 * @return              data pointer passed to QBXHTMLQuickVisitor constructor
 */
extern void *
QBXHTMLQuickVisitorGetData(QBXHTMLQuickVisitor self);

/**
 * Setter functions for general type node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a general node type
 **/
extern void
QBXHTMLQuickVisitorSetVisitableCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for div node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a div node
 **/
extern void
QBXHTMLQuickVisitorSetDivCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for p node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a p node
 **/
extern void
QBXHTMLQuickVisitorSetPCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for html node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a html node
 **/
extern void
QBXHTMLQuickVisitorSetHtmlCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for head node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a head node
 **/
extern void
QBXHTMLQuickVisitorSetHeadCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for title node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a title node
 **/
extern void
QBXHTMLQuickVisitorSetTitleCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for body node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a body node
 **/
extern void
QBXHTMLQuickVisitorSetBodyCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for font node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a font node
 **/
extern void
QBXHTMLQuickVisitorSetFontCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for img node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a img node
 **/
extern void
QBXHTMLQuickVisitorSetImgCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for input node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a input node
 **/
extern void
QBXHTMLQuickVisitorSetInputCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for select node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a select node
 **/
extern void
QBXHTMLQuickVisitorSetSelectCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for option node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a option node
 **/
extern void
QBXHTMLQuickVisitorSetOptionCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for form node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a form node
 **/
extern void
QBXHTMLQuickVisitorSetFormCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for button node callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a button node
 **/
extern void
QBXHTMLQuickVisitorSetButtonCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * Setter functions for text callback.
 * Use this or any other callback setter function to add functionality to QuickVisitor.
 * If You don't set any callback then quick visitor will be useless.
 *
 * @param[in] self      quick visitor reference
 * @param[in] callback  callback to be called when visiting a text
 **/
extern void
QBXHTMLQuickVisitorSetTextCallback(QBXHTMLQuickVisitor self, QBXHTMLVisitVisitableNode callback);

/**
 * @}
 **/

#endif //QB_XHTML_QUICK_VISITOR_H

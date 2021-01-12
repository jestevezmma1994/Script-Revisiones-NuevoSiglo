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

#ifndef QB_XHTML_VISITOR_H
#define QB_XHTML_VISITOR_H

/**
 * @file QBXHTMLVisitor.h XHTML tree nodes visitor interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBXHTMLWidgets/QBXHTMLNodes.h>
#include <QBDataModel3/QBActiveTree.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBXHTMLVisitor XHTML tree nodes visitor interface
 * @ingroup GUI
 * @{
 *
 * This is the interface that needs to be implemented in order to interact with an XHTML document tree.
 * There are also some deafult visitation methods here as well as all tree and node visitition methods.
 **/

/** @{
 * Noop visitation Functions that will perform no operation.
 * Use those functions if your visitor is not interested in visiting a particular
 * type of node.
 */

/** Noop visit method*/
void QBNodeVisitorNoopVisitable(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopDiv(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopP(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopHtml(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopHead(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopTitle(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopBody(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopFont(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopImg(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopInput(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopSelect(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopOption(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopForm(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopButton(SvObject self, QBXHTMLVisitableNode node);
/** Noop visit method*/
void QBNodeVisitorNoopText(SvObject self, QBXHTMLVisitableNode node);
/** @} */

/** @{
 * Print visitation Functions that will print information on the node beeing visited.
 * The node type and value of "id" attribute is printed for each node.
 * Use those functions if your visitor just wants to print info on visited objects
 * to console.
 */

/** Print node info visit method*/
void QBNodeVisitorPrintVisitable(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintDiv(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintP(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintHtml(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintHead(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintTitle(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintBody(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintFont(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintImg(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintInput(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintSelect(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintOption(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintForm(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintButton(SvObject self, QBXHTMLVisitableNode node);
/** Print node info visit method*/
void QBNodeVisitorPrintText(SvObject self, QBXHTMLVisitableNode node);
/** @} */

/** @{
 * Upcast visitation Functions that will visit node as if this node was an instance of its superclass.
 * In curent implementation of XHTML nodes all nodes ar derived from QBXHTMLVisitableNode, so in case of
 * using one of below functions you need to implement visitation method for QBXHTMLVisitableNode.
 * This functions will be usefull if you want to treat a group, or all the node types in the same manner.
 * In example if you want to print the value of "width" attribute for each node you can implement this functionality
 * in QBXHTMLVisitableNode visit function, and use below Upcast functions for other types of nodes.
 */

/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastDiv(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastP(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastHtml(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastHead(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastTitle(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastBody(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastFont(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastImg(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastInput(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastSelect(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastOption(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastForm(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastButton(SvObject self, QBXHTMLVisitableNode node);
/** Upcast the node to QBXHTMLVisitableNode and visit it*/
void QBNodeVisitorUpcastText(SvObject self, QBXHTMLVisitableNode node);
/** @} */


/**
 * Visitation method for QBXHTMLVisitableNode
 * @param [in] self     the visitor handle
 * @param [in] node     the node being visited
 **/
typedef void (*QBXHTMLVisitVisitableNode)(SvObject self, QBXHTMLVisitableNode node);

/**
 * QBNodeVisitor interface.
 * This interface is responsible for visiting XHTML document tree nodes.
 * Objects implementing this interface are capable of visiting the XHTML document tree.
 * This interface only focuses on "What to do" aspect of tree node visitation. To choose the order
 * of visitation you should use one of @c QBNodeVisitorVisit... methods
 *
 * @interface QBNodeVisitor
 **/
typedef const struct QBNodeVisitor_ {
    /** general Visitable node visitation method */
    QBXHTMLVisitVisitableNode visitVisitableNode;
    /** Div node visitation method */
    QBXHTMLVisitVisitableNode visitDivNode;
    /** P node visitation method */
    QBXHTMLVisitVisitableNode visitPNode;
    /** Html node visitation method */
    QBXHTMLVisitVisitableNode visitHtmlNode;
    /** Head node visitation method */
    QBXHTMLVisitVisitableNode visitHeadNode;
    /** Title node visitation method */
    QBXHTMLVisitVisitableNode visitTitleNode;
    /** Body node visitation method */
    QBXHTMLVisitVisitableNode visitBodyNode;
    /** Font node visitation method */
    QBXHTMLVisitVisitableNode visitFontNode;
    /** Img node visitation method */
    QBXHTMLVisitVisitableNode visitImgNode;
    /** Input node visitation method */
    QBXHTMLVisitVisitableNode visitInputNode;
    /** Select node visitation method */
    QBXHTMLVisitVisitableNode visitSelectNode;
    /** Option node visitation method */
    QBXHTMLVisitVisitableNode visitOptionNode;
    /** Form node visitation method */
    QBXHTMLVisitVisitableNode visitFormNode;
    /** Button node visitation method */
    QBXHTMLVisitVisitableNode visitButtonNode;
    /** Text node visitation method */
    QBXHTMLVisitVisitableNode visitTextNode;
} *QBNodeVisitor;

/**
 * Get the object representing QBNodeVisitor type.
 *
 * @return      QBNodeVisitor type meta object
 **/
extern SvInterface
QBNodeVisitor_getInterface(void);

/**
 * Tree postorder visitation method.
 * All nodes of a tree will be visited by a given visitor in postorder traversal
 *
 * @param [in] visitor  the visitor to visit the tree
 * @param [in] tree     the tree to be visited
 **/
extern void
QBNodeVisitorVisitTreePostorder(SvObject visitor, QBActiveTree tree);

/**
 * Tree preorder visitation method.
 * All nodes of a tree will be visited by a given visitor in preorder traversal
 *
 * @param [in] visitor  the visitor to visit the tree
 * @param [in] tree     the tree to be visited
 **/
extern void
QBNodeVisitorVisitTreePreorder(SvObject visitor, QBActiveTree tree);

/**
 * Single node visitation method.
 * Only the node passed as parameter will bi visited.
 *
 * @param [in] visitor  the visitor to visit the node
 * @param [in] node     the node to be visited
 **/
extern void
QBNodeVisitorVisitNode(SvObject visitor, QBXHTMLVisitableNode node);

/**
 * Node children visitation method.
 * All child nodes of a given node will be visited. The children will be visited in order in which they
 * were added to a tree (same order as in XHTML document). Only node children will be visited.
 * The node itself will not be visited nor will any child children.
 *
 * @param [in] visitor  the visitor to visit the node children
 * @param [in] node     the node whose children are to be visited
 **/
extern void
QBNodeVisitorVisitNodeChildren(SvObject visitor, QBXHTMLVisitableNode node);

/**
 * Node subtree postorder visitation method.
 * The node itself as well as all its descendants will be visited in postorder.
 *
 * @param [in] visitor  the visitor to visit the node subtree
 * @param [in] node     the node whose subtree is to be visited
 **/
extern void
QBNodeVisitorVisitNodeSubtreePostorder(SvObject visitor, QBXHTMLVisitableNode node);

/**
 * Node subtree preorder visitation method.
 * The node itself as well as all its descendants will be visited in preorder.
 *
 * @param [in] visitor  the visitor to visit the node subtree
 * @param [in] node     the node whose subtree is to be visited
 **/
extern void
QBNodeVisitorVisitNodeSubtreePreorder(SvObject visitor, QBXHTMLVisitableNode node);

/**
 * Node parent visitation method.
 * Only the node parent (if any) will be visited, the node itself will not.
 *
 * @param [in] visitor  the visitor to visit the node parent
 * @param [in] node     the node whose parent is to be visited
 **/
extern void
QBNodeVisitorVisitNodeParent(SvObject visitor, QBXHTMLVisitableNode node);

/**
 * Node ancestors visitation method.
 * The nodes straight line ancestors will be visited (parent, grandparent, great-grandparent, and so on).
 * The node itself will not be visited.
 *
 * @param [in] visitor  the visitor to visit the node ancestors
 * @param [in] node     the node whose ancestors are to be visited
 **/
extern void
QBNodeVisitorVisitNodeAncestors(SvObject visitor, QBXHTMLVisitableNode node);

/**
 * QBPrintNodeVisitor print node info visitor.
 * Example visitor (yet still usefull for debuging) that will just print info on visited nodes.
 * The visitor makes use of all @c QBNodeVisitorPrint... methods.
 *
 * @class QBPrintNodeVisitor
 * @extends SvObject
 * @implements QBNodeVisitor
 **/
typedef struct QBPrintNodeVisitor_ *QBPrintNodeVisitor;

/**
 * Create new instance of QBPrintNodeVisitor.
 *
 * @param [out] errorOut    error output
 * @return                  new instance of QBPrintNodeVisitor or @c NULL in case of error
 **/
extern QBPrintNodeVisitor
QBPrintNodeVisitorCreate(SvErrorInfo *errorOut);

/**
 * QBFindNodeVisitor find node visitor.
 * Visitor for finding a node in a tree structure. The node can be searched by its type, its id or both.
 * The first node that will match search criteria during visitation will be remembered as a result.
 * To get search result use @c QBFindNodeVisitorGetResult after visitation.
 *
 * @class QBFindNodeVisitor
 * @extends SvObject
 * @implements QBNodeVisitor
 **/
typedef struct QBFindNodeVisitor_ *QBFindNodeVisitor;

/**
 * Create new instance of QBFindNodeVisitor.
 *
 * @param [in] nodeType     type of node to find or @c NULL if not relevant
 * @param [in] nodeId       value of "id" attribute to find or @c NULL if not relevant
 * @param [out] errorOut    error output
 **/
extern QBFindNodeVisitor
QBFindNodeVisitorCreate(SvString nodeType, SvString nodeId, SvErrorInfo *errorOut);

/**
 * Get the search result.
 * Remember to call this function after a tree, a subtree or some other set of nodes has been visited by the visitor.
 *
 * @param [in] self     this visitor handle
 * @return              The node that was found or @c NULL in case of no match
 */
extern QBXHTMLVisitableNode
QBFindNodeVisitorGetResult(QBFindNodeVisitor self);

/**
 * Helper function to perform a node search on a tree using @c QBFindNodeVisitor.
 * All this function does is to create a @c QBFindNodeVisitor visitor with given parameters,
 * perform a postorder tree traversal using this visitor, and return a search result.
 *
 * @param [in] tree         tree to perform node search on
 * @param [in] nodeType     type of node to find or @c NULL if not relevant
 * @param [in] nodeId       value of "id" attribute to find or @c NULL if not relevant
 * @param [out] errorOut    error output
 * @return                  The node that was found or @c NULL in case of no match or any error
 **/
extern QBXHTMLVisitableNode
QBXHTMLWidgetsFindNode(QBActiveTree tree, SvString nodeType, SvString nodeId, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif //QB_XHTML_VISITOR_H

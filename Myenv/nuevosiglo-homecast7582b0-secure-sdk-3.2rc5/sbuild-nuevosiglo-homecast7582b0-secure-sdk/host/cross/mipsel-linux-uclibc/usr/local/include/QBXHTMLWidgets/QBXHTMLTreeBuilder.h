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

#ifndef QB_XHTML_TREE_BUILDER_H
#define QB_XHTML_TREE_BUILDER_H

/**
 * @file QBXHTMLTreeBuilder.h XHTML Tree represenation builder
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDataModel3/QBActiveTree.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBXHTMLTreeBuilder XHTML tree builder
 * @ingroup GUI
 * @{
 *
 * A class parsing XML and building its tree representation
 **/

/**
 * QBXHTMLTreeBuilder XHTML tree builder class.
 * Just create it, feed it with XML and get the result as QBActiveTree.
 *
 * @class QBXHTMLTreeBuilder
 * @extends SvObject
 **/
typedef struct QBXHTMLTreeBuilder_ *QBXHTMLTreeBuilder;

/**
 * Create a tree builder.
 *
 * @param[out] errorOut     error output
 * @return                  new tree builder or @c NULL in case of error
 **/
extern QBXHTMLTreeBuilder
QBXHTMLTreeBuilderCreate(SvErrorInfo *errorOut);

/**
 * Feed the tree builder with a portion of XML data passed as plain char buffer.
 * This function can be called multiple times with partial XML data. The only important thing is that
 * you eventually pass all the XML data and do it in order.
 *
 * @param[in] self          this builder
 * @param[in] buffer        buffer containig next part of XML document
 * @param[in] length        length of data in the buffer
 **/
extern void
QBXHTMLTreeBuilderFeedDataRaw(QBXHTMLTreeBuilder self, const char *buffer, size_t length);

/**
 * Feed the tree builder with a portion of XML data passed as SvString.
 * This function can be called multiple times with partial XML data. The only important thing is that
 * you eventually pass all the XML data and do it in order.
 *
 * @param[in] self          this builder
 * @param[in] data          next part of XML document
 **/
extern void
QBXHTMLTreeBuilderFeedData(QBXHTMLTreeBuilder self, SvString data);

/**
 * Get the tree representing document passed to builder with Feed functions.
 * This function can be called only after the whole document was passed to the builder with Feed functions.
 * The returned tree is an QBActiveTree.
 *
 * WARNING! All the nodes of this tree are instances of QBActiveTreeExtendedNode.
 * Each Node of the tree has a private data of type QBXHTMLVisitableNode or one of its derived classes.
 * This construct was introduced because QBActiveTreeNode cannot be derived from outside of ActiveTree library.
 *
 * For each node in the XML document one node of the tree is created (plus nodes for the text).
 * To perform any operations on the document tree appropriate visitor should be created. See @c QBXHTMLVisitor for details
 *
 * @param[in] self          this builder
 * @param[out] errorOut     error output
 * @return                  tree representing XML document or @c NULL in case of error
 **/
extern QBActiveTree
QBXHTMLTreeBuilderGetResult(QBXHTMLTreeBuilder self, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif //QB_XHTML_TREE_BUILDER_H

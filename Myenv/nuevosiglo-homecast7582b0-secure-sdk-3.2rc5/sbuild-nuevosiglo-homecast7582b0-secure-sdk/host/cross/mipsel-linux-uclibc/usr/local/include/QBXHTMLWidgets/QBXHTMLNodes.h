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

#ifndef QB_XHTML_NODES_H
#define QB_XHTML_NODES_H

/**
 * @file QBXHTMLNodes.h XHTML Tree nodes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBXHTMLNodes XHTML tree nodes
 * @ingroup GUI
 * @{
 *
 * Nodes class that represent tags in XHTML document
 **/

/**
 * QBXHTMLVisitableNode General visitable node.
 * This is a base class for all other nodes in XHTML document
 *
 * @class QBXHTMLVisitableNode
 * @extends SvObject
 * @implements QBXHTMLVisitable
 **/
typedef struct QBXHTMLVisitableNode_ *QBXHTMLVisitableNode;

/**
 * Get the value of attribute from the XML document.
 * WARNING! html documents are case insensitive, so case in name will not matter.
 *
 * @param [in] self         this object reference
 * @param [in] name         name of the attribute to retrieve
 * @return                  value of a given attribute or @c NULL if attribute was not specified for this node.
 **/
extern SvString
QBXHTMLVisitableNodeGetAttribute(QBXHTMLVisitableNode self, const char *name);

/**
 * Possible sufixes for numeric values of attribute.
 **/
typedef enum {
    /** no sufiv (like "100") */
    QBXHTMLVisitableNodeIntAttributeSufix_Clear,
    /** px sufix indicating pixels (like "100px") */
    QBXHTMLVisitableNodeIntAttributeSufix_Px,
    /** % sufix (like "100%") */
    QBXHTMLVisitableNodeIntAttributeSufix_Percentage,
    /** any other sufix (like "100$" or "12hpa") */
    QBXHTMLVisitableNodeIntAttributeSufix_Other
} QBXHTMLVisitableNodeIntAttributeSufix;

/**
 * Get the value of attribute from the XML document, and convert it to int value.
 * WARNING! html documents are case insensitive, so case in name will not matter.
 * If the value is followed by sufix it will also be read properly.
 * SufixOut indicates what type of sufix was found following value
 *
 * @param [in] self         this object reference
 * @param [in] name         name of the attribute to retrieve
 * @param [in] defaultValue value that will be returned from this function if attribute is not specified for this node
 * @param [out] sufixOut    sufix given to the attribute
 * @return                  value of a given attribute or @c defaultValue if attribute was not specified for this node or is not a number.
 **/
extern long long
QBXHTMLVisitableNodeGetAttributeAsInt(QBXHTMLVisitableNode self, const char *name, long long defaultValue, QBXHTMLVisitableNodeIntAttributeSufix *sufixOut);

/**
 * @}
 **/

#endif //QB_XHTML_NODES_H

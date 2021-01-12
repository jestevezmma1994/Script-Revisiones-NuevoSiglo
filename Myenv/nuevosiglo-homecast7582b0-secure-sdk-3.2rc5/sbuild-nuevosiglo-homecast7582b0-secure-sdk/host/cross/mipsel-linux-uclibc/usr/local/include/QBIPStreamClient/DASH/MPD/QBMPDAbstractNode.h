/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBMPDABSTRACTNODE_H_
#define QBMPDABSTRACTNODE_H_

/**
 * @file QBMPDAbstractNode.h
 * @brief Abstract node API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAbstractAttributes.h>

/**
 * @defgroup QBMPDAbstractNode Abstract node class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Abstract node class.
 * @class QBMPDAbstractNode
 * @extends SvObject
 **/
typedef struct QBMPDAbstractNode_ *QBMPDAbstractNode;

/**
 * Get runtime type identification object representing
 * type of Abstract node class.
 *
 * @return Abstract node class
 **/
extern SvType
QBMPDAbstractNode_getType(void);

/**
 * Get name of node.
 *
 * @param[in] self Abstract node handle
 *
 * @return name, @c NULL in case of error
 **/
extern SvString
QBMPDAbstractNodeGetName(QBMPDAbstractNode self);

/**
 * Get value of node.
 *
 * @param[in] self Abstract node handle
 *
 * @return value, @c NULL in case of error
 **/
extern SvString
QBMPDAbstractNodeGetValue(QBMPDAbstractNode self);

/**
 * Get attributes of node.
 *
 * @param[in] self Abstract node handle
 *
 * @return attributes, @c NULL in case of error
 **/
extern QBMPDAbstractAttributes
QBMPDAbstractNodeGetAttributes(QBMPDAbstractNode self);

/**
 * @}
 **/

#endif /* QBMPDABSTRACTNODE_H_ */

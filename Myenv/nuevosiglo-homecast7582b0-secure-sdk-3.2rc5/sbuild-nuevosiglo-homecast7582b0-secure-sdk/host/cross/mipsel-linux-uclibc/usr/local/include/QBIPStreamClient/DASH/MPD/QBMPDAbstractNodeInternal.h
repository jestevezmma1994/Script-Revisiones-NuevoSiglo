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

#ifndef QBMPDABSTRACTNODEINTERNAL_H_
#define QBMPDABSTRACTNODEINTERNAL_H_

/**
 * @file QBMPDAbstractNode.h
 * @brief Abstract node private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAbstractAttributes.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAbstractNode.h>

/**
 * @addtogroup QBMPDAbstractNode
 * @{
 **/

/**
 * Create extension node.
 *
 * @param[in]  name     name for node
 * @param[out] errorOut error info
 * @return              new instance of node, @c NULL in case of error
 **/
extern QBMPDAbstractNode
QBMPDAbstractNodeCreate(SvString name,
                        SvErrorInfo *errorOut);

/**
 * Set value for extension node.
 *
 * @param[in]  self     Abstract node handle
 * @param[in]  value    value for node
 * @param[out] errorOut error info
 **/
extern void
QBMPDAbstractNodeSetValue(QBMPDAbstractNode self,
                          SvString value,
                          SvErrorInfo *errorOut);

/**
 * Set attributes for extension node.
 *
 * @param[in]  self       Abstract node handle
 * @param[in]  attributes attributes for node
 * @param[out] errorOut   error info
 **/
extern void
QBMPDAbstractNodeSetAttributes(QBMPDAbstractNode self,
                               QBMPDAbstractAttributes attributes,
                               SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDABSTRACTNODEINTERNAL_H_ */

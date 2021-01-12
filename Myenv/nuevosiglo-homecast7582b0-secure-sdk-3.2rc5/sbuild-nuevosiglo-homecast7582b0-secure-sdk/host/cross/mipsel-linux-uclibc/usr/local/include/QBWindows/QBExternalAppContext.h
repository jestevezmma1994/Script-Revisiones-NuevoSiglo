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

#ifndef QBEXTERNALAPPCONTEXT_H_
#define QBEXTERNALAPPCONTEXT_H_

#include <QBWindowContext.h>

#include <QBWindows/QBExternalAppLogic.h>

/**
 * @file QBExternalAppContext.h
 * @brief QBExternalAppContext is context displaying and managing external application.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBExternalAppContext class.
 * @{
 **/

/**
 * Pointer to QBExternalAppContext type definition
 */
typedef struct QBExternalAppContext_ *QBExternalAppContext;

/**
 * External application context class definition
 */
struct QBExternalAppContext_ {
    struct QBWindowContext_t super_;     /**< base type */
    QBExternalAppLogic extAppLogic;     /**< external application logic */
    SvString appName;                   /**< external application name */
};

/**
 * Virtual methods of the external application window context class.
 **/
typedef const struct QBExternalAppContextVTable_ {
    struct QBWindowContextVTable_ super_; /**< virtual methods of the base class */

    /**
     * Close application.
     *
     * @param[in] ctx_      window context handle
     **/
    void (*close)(QBWindowContext ctx_);
} *QBExternalAppContextVTable;

/**
 * This function returns QBExternalAppContext type.
 *
 * @return                      QBExternalAppContext type
 */
SvType QBExternalAppContext_getType(void);

/**
 * This function closes QBExternalAppContext.
 *
 * @param[in] ctx_              QBExternalAppContext handle
 */
void QBExternalAppContextClose(QBWindowContext ctx_);

/**
 * This function sets external application name.
 *
 * @param[in] ctx_              QBExternalAppContext handle
 * @param[in] appName           external application name
 */
void QBExternalAppContextSetName(QBWindowContext ctx_, SvString appName);

/**
 * This function returns external application name.
 *
 * @param[in] ctx_              QBExternalAppContext handle
 *
 * @return                      SvString application name
 */
SvString QBExternalAppContextGetName(QBWindowContext ctx_);

/**
 * @}
 **/
#endif /* QBEXTERNALAPPCONTEXT_H_ */

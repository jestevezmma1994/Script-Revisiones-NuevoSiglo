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

#ifndef QB_CONAX_MAIL_MANAGER_H_
#define QB_CONAX_MAIL_MANAGER_H_

/**
 * @file QBConaxMailManager.h Conax mail manager
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <main_decl.h>

#include <stddef.h>

/**
 * @defgroup QBConaxMailManager Conax mail manager
 * @ingroup CubiTV_services
 * @{
 */

/**
 * QBConax mail manager listener.
 **/
typedef const struct QBConaxMailManagerListener_ {
    /**
     * Notification about mail state change
     *
     * @param[in] self_                 handle to object implementing QBConaxMailManagerListener interface
     * @param[in] mailCount             mail count
     * @param[in] unreadMailsCount      unread mail count
     **/
    void (*mailChanged)(SvObject self_,
                        size_t mailCount,
                        size_t unreadMailCount);
} *QBConaxMailManagerListener;


/**
 * Get runtime type identification object representing
 * QBConaxMailManagerListener interface.
 *
 * @return QBConaxMailManagerListener interface object
**/
SvInterface
QBConaxMailManagerListener_getInterface(void);

/**
 * Conax menu item choice class.
 *
 * @class QBConaxMailManager QBConaxMailManager.h <Services/QBConaxMailManager.h>
 * @extends SvObject
 **/
typedef struct QBConaxMailManager_ *QBConaxMailManager;

/**
 * Create new Conax mail manager.
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] appGlobals    application globals
 * @param[out] *errorOut    error info
 * @return    New Conax mail manager or @c NULL in case of error
 **/
QBConaxMailManager
QBConaxMailManagerCreate(AppGlobals appGlobals, SvErrorInfo *errorOut);

/**
 * Start Conax mail manager.
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] self      Conax mail manager handle
 **/
void
QBConaxMailManagerStart(QBConaxMailManager self);

/**
 * Stop Conax mail manager.
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] self      Conax mail manager handle
 **/
void
QBConaxMailManagerStop(QBConaxMailManager self);

/**
 * Add listener to Conax mail manager.
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] self      Conax mail manager handle
 * @param[in] listener  listener handle
 **/
void
QBConaxMailManagerAddListener(QBConaxMailManager self, SvObject listener);

/**
 * Add listener to Conax mail manager.
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] self      Conax mail manager handle
 * @param[in] mail      mail handle
 **/
void
QBConaxMailManagerMarkMailOpened(QBConaxMailManager self, SvObject mail);

/**
 * Is mail opened
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] self      Conax mail manager handle
 * @param[in] mail      mail handle
 * @return   is mail opened
 **/
bool
QBConaxMailManagerIsMailOpened(QBConaxMailManager self, SvObject mail);

/**
 * Get mail text
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] self      Conax mail manager handle
 * @param[in] mail      mail handle
 * @return   mail text string
 **/
SvString
QBConaxMailManagerGetMailText(QBConaxMailManager self, SvObject mail);

/**
 * Get mail slot
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] self      Conax mail manager handle
 * @param[in] mail      mail handle
 * @return   mail slot
 **/
int
QBConaxMailManagerGetMailSlot(QBConaxMailManager self, SvObject mail);

/**
 * Get unread mail count
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] self      Conax mail manager handle
 * @return   mail slot
 **/
size_t
QBConaxMailManagerGetUnreadMailCount(QBConaxMailManager self);

/**
 * Get mail from node
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] node     node from menu containing mail
 * @return   mail handle
 **/
SvObject
QBConaxMailManagerGetMailFromNode(QBActiveTreeNode node);

/**
 * Is node 'No Messages' node
 *
 * @public @memberof QBConaxMailManager
 *
 * @param[in] node     node from menu
 * @return   is node no messages
 **/
bool
QBConaxMailManagerIsNodeNoMessages(QBActiveTreeNode node);

/**
 * @}
 **/

#endif /* QB_CONAX_MAIL_MANAGER_H_ */

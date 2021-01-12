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

#ifndef QB_SUBTREE_NOTIFIER_H_
#define QB_SUBTREE_NOTIFIER_H_

/**
 * @file  QBSubtreeNotifier.h
 * @brief Main menu focus change notification service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvInterface.h>

/**
 * @defgroup QBSubtreeNotifier Main menu focus monitor class
 * @ingroup  CubiTV_services
 **/

/**
 * QBSubtreeNotifier is a service used to monitor the focus of main menu.
 * It can be used to perform some heavy work only when the user ventures somewhere
 * into the menu. In example there can be a menu that will have some submenu
 * entries downloaded from the server. We can postpone the server communication
 * to the time when a parent menu gets focused. This way if the menu is never choosen
 * we will not bother the server at all, and at least we will not do it as soon
 * as the box starts.
 *
 * Warning! This service is designed to notify listeners when subtree gets focused.
 * This means that for the following menu layout:
 *
 * -entry1
 *  -subentry1
 *  -subentry2
 *  -subentry3
 * -entry2
 *  -subentry4
 *  -subentry5
 *
 * When the user focuses on "subentry1", all the listeners registered on path "entry1" will be
 * notified, as "subentry1" is part of "entry1" subtree.  If subentry2 and subentry3 needs to perform
 * some work in order to be fully functional their listeners has to be added to the notifier on path
 * "entry1" and NOT "entry1/subentry2" and "entry1/subentry3" respectively. For "subentry4" amd 5 the
 * correct path would of course be "entry2"
 * @{
 **/
typedef struct QBSubtreeNotifier_t *QBSubtreeNotifier;

/**
 * Create Subtree Notifier monitor service.
 *
 * @return                  new Subtree Notifier monitor service instance
 **/
extern QBSubtreeNotifier
QBSubtreeNotifierCreate(void);

/**
 * Add an object to the list of focus listeners for menu node found under path.
 *
 * @param[in] self      this object
 * @param[in] path      the path where menu entry that is to be observed can be found
 * @param[in] listener  the object implementing @c QBSubtreeNotifierListener interface
 * @return              0 on success and -1 on error
 **/
extern int
QBSubtreeNotifierAddListener(QBSubtreeNotifier self, SvGenericObject path, SvGenericObject listener);

/**
 * Remove an object from the list of focus listeners for menu node found under path.
 *
 * @param[in] self      this object
 * @param[in] path      the path where observed menu entry can be found
 * @param[in] listener  the object registered as a listener
 * @return              0 on success and -1 on error
 **/
extern int
QBSubtreeNotifierRemoveListener(QBSubtreeNotifier self, SvGenericObject path, SvGenericObject listener);

/**
 * QBSubtreeNotifierListener interface for listening on main menu focus change.
 *
 * @interface QBSubtreeNotifierListener
 **/
typedef struct QBSubtreeNotifierListener_t {
    /**
     * Method called when observed menu subtree gets focused.
     *
     * @param [in] self     the listener object.
     **/
    void (* subtreeEntered)(SvGenericObject self);

    /**
     * Method called when observed menu subtree looses focus.
     *
     * @param [in] self     the listener object.
     **/
    void (* subtreeLeft)(SvGenericObject self);
} *QBSubtreeNotifierListener;

/**
 * Get the object representing QBSubtreeNotifierListener type.
 *
 * @return      QBSubtreeNotifierListener type meta object
 **/
extern SvInterface
QBSubtreeNotifierListener_getInterface(void);

/**
 * @}
 **/


#endif


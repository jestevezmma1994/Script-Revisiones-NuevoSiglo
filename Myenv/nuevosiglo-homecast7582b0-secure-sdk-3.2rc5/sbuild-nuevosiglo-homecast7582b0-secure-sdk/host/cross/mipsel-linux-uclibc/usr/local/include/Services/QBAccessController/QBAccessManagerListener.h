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

/* App/Libraries/QBAccessController/QBAccessManagerListener.h */

#ifndef QB_ACCESS_MANAGER_LISTENER_H_
#define QB_ACCESS_MANAGER_LISTENER_H_

/**
 * @file QBAccessManagerListener.h
 * @brief Access Manager Listener Interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBAccessManagerListener Access manager listener interface
 * @ingroup QBAccessController
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <Services/QBAccessController/QBAccessManager.h>


/**
 * Get runtime type identification object representing
 * access manager listener interface.
 * @return Access manager listener interface identification object
 **/
extern SvInterface
QBAccessManagerListener_getInterface(void);


/**
 * QBAccessManagerListener interface.
 **/
typedef struct QBAccessManagerListener_t *QBAccessManagerListener;
struct QBAccessManagerListener_t {
   /**
    * Notify that authenticated user had changed.
    *
    * This method is called when authentication is revoked explicitly
    * and also when asynchronous authentication is finished (regardless
    * whether user's authentity has been confirmed or not)
    *
    * @param[in] self_      listener handle
    * @param[in] manager    access manager handle
    * @param[in] domainName domain with the changed status
    **/
   void (*authenticationChanged)(SvGenericObject self_,
                                 QBAccessManager manager,
                                 SvString domainName);

};


/**
 * @}
 **/

#endif

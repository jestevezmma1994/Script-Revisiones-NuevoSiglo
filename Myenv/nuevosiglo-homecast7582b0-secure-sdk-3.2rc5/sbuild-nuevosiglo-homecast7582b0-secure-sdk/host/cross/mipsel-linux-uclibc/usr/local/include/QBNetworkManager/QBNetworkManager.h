/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_NETWORK_MANAGER_H_
#define QB_NETWORK_MANAGER_H_

/**
 * @file QBNetworkManager.h Netwok Manager API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBNetworkManager NetworkManager
 * @{
 **/

/**
 * Network Manager class is a networking core. It manages
 * interfaces handlers of different types, receives IPC communication
 * from external processes and notifies external processes about changes
 * on the network interfaces.
 *
 * @class QBNetworkManager
 **/
typedef struct QBNetworkManager_ *QBNetworkManager;

/**
 * Creates a new instance of Network Manager.
 *
 * @return a new instance of QBNetlinkListener
 **/
extern QBNetworkManager
QBNetworkManagerCreate(void);

/**
 * Destroys an instance of Network Manager.
 *
 * @param[in] self QBNetworkManager handle to be destroyed
 **/
void
QBNetworkManagerDestroy(QBNetworkManager self);

/**
 * Starts an instance of Network Manager.
 *
 * @param[in] self QBNetworkManager handle to be started
 **/
extern void
QBNetworkManagerStart(QBNetworkManager self);

/**
 * Stops an instance of Network Manager.
 *
 * @param[in] self QBNetworkManager handle to be stopped
 **/
extern void
QBNetworkManagerStop(QBNetworkManager self);

/**
 * @}
**/

#endif //QB_NETWORK_MANAGER_H_

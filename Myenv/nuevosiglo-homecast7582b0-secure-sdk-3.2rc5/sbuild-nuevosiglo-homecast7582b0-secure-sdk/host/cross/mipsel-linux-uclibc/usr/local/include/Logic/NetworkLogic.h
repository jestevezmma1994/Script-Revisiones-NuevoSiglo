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

#ifndef NETWORK_LOGIC_H_
#define NETWORK_LOGIC_H_

/**
 * @file NetworkLogic.h Network logic class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Logic/InitLogic.h>
#include <Services/core/QBTextRenderer.h>
#include <Services/core/QBNetworkWatcher.h>
#include <QBApplicationController.h>
#include <QBNetworkMonitor.h>
#include <QBNetworkManager/Utils/QBNetworkManagerTypes.h>
#include <QBNetManager.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * @defgroup NetworkLogic Network logic class
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Network logic class.
 * @class QBNetworkLogic NetworkLogic.h <Logic/NetworkLogic.h>
 *
 * @extends SvObject
 *
 * This is an application logic class implementing functionality
 * related to networking requirements. An instance of this
 * class can be created by using @ref QBLogicFactory.
 **/
typedef struct QBNetworkLogic_ *QBNetworkLogic;

/**
 * Network logic virtual methods table.
 **/
typedef const struct QBNetworkLogicVTable_ {
    /// virtual methods table of the base class
    struct SvObjectVTable_ super_;

    /**
     * Initialize network logic instance.
     *
     * @param[in] self          network logic handle
     * @param[out] netManager   net manager handle
     * @param[out] errorOut     error info
     * @return                  @a self or @c NULL in case of error
     **/
    QBNetworkLogic (*init)(QBNetworkLogic self,
                           QBNetManager *netManager,
                           SvErrorInfo *errorOut);

    /**
     * Start network logic.
     *
     * @param[in] self          network logic handle
     * @param[in] self          QBNetworkMonitor handle
     **/
    void (*start)(QBNetworkLogic self);

    /**
     * Stop network logic.
     *
     * @param[in] self          network logic handle
     **/
    void (*stop)(QBNetworkLogic self);

    /**
     * Returns requirements for network watcher
     *
     * @param[in] self      network logic handle
     * @param[in] vpnType   VPN type
     * @return              QBNetworkWatcherRequirements handle
     **/
    QBNetworkWatcherRequirements (*getRequirements)(QBNetworkLogic self,
                                                    QBNetworkManagerVPNType vpnType);
} *QBNetworkLogicVTable;


/**
 * Get runtime type identification object representing network logic class.
 * @return network logic class
 **/
extern SvType
QBNetworkLogic_getType(void);


/**
 * Create an instance of network logic using @ref QBLogicFactory.
 *
 * @memberof QBNetworkLogic
 *
 * @param[in] res               application handle
 * @param[in] scheduler         application scheduler handle
 * @param[in] initLogic         init logic handle
 * @param[in] controller        application controller handle
 * @param[in] textRenderer      text renderer handle
 * @param[in] networkMonitor    network monitor handle
 * @param[out] netManager       net manager handle
 * @param[out] errorOut         error info
 * @return                      created network logic, @c NULL in case of error
 **/
extern QBNetworkLogic
QBNetworkLogicCreate(SvApplication res,
                     SvScheduler scheduler,
                     QBInitLogic initLogic,
                     QBApplicationController controller,
                     QBTextRenderer textRenderer,
                     QBNetworkMonitor networkMonitor,
                     QBNetManager *netManager,
                     SvErrorInfo *errorOut);

/**
 * Start network logic.
 *
 * @memberof QBNetworkLogic
 *
 * @param[in] self      network logic handle
 * @param[in] self      QBNetworkMonitor handle
 **/
static inline void
QBNetworkLogicStart(QBNetworkLogic self)
{
    return SvInvokeVirtual(QBNetworkLogic, self, start);
}

/**
 * Stop network logic.
 *
 * @memberof QBNetworkLogic
 *
 * @param[in] self      network logic handle
 **/
static inline void
QBNetworkLogicStop(QBNetworkLogic self)
{
    return SvInvokeVirtual(QBNetworkLogic, self, stop);
}

/**
 * @}
 **/


#endif /* NETWORK_LOGIC_H_ */

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUBIWAREMWCLIENTSERVICE_H_
#define CUBIWAREMWCLIENTSERVICE_H_

/**
 * @file    CubiwareMWClientService.h
 * @brief   MW client service.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Services/core/QBMiddlewareManager.h>

/**
 * @defgroup MW client service
 * @ingroup CubiwareMWClientService
 * @{
 **/

/**
 * MW client service enable to add new plugins which need communication with MW.
 **/
typedef struct CubiwareMWClientServicePluginInterface_ {
    /**
     * Start connected plugins.
     *
     * When CubiwareMWClientService is starting, it also starts all registered plugins.
     *
     * @param[in]  self_        handle to an object implementing @ref CubiwareMWClientServicePluginInterface
     * @param[in]  serverInfo   server info class
     * @param[in]  customerID   customer ID
     **/
    void (* start)(SvObject self_, SvXMLRPCServerInfo serverInfo, SvString customerId);

    /**
     * Stop connected plugins.
     *
     * When CubiwareMWClientService is stopping, it also stops all registered plugins.
     *
     * @param[in]  self_        handle to an object implementing @ref CubiwareMWClientServicePluginInterface
     **/
    void (* stop)(SvObject self_);

    /**
     * Notify listeners that data have been changed.
     *
     * @param[in]  self_        handle to an object implementing @ref CubiwareMWClientServicePluginInterface
     * @param[in]  serverInfo   server info class
     * @param[in]  customerID   customer ID
     **/
    void (* middlewareDataChanged)(SvObject self_, SvXMLRPCServerInfo serverInfo, SvString customerId);
} *CubiwareMWClientServicePluginInterface;

/**
 * Get runtime type identification object representing
 * CubiwareMWClientServicePluginInterface.
 *
 * @return CubiwareMWClientServicePluginInterface interface object
 **/
extern SvInterface
CubiwareMWClientServicePluginInterface_getInterface(void);

typedef struct CubiwareMWClientService_ *CubiwareMWClientService;

/**
 * Create MW Client Service.
 *
 * @param[in] middlewareManager    QBMiddlewareManager handle
 * @return                  new MW client service instance or @c NULL
 *                          in case of error
 **/
extern CubiwareMWClientService
CubiwareMWClientServiceCreate(QBMiddlewareManager middlewareManager);

/**
 * Start MW Client Service.
 *
 * @param[in] self      MW Client Servicer handle
 */
SvExport void
CubiwareMWClientServiceStart(CubiwareMWClientService self);

/**
 * Stop MW Client Service.
 *
 * @param[in] self      config monitor handle
 */
SvExport void
CubiwareMWClientServiceStop(CubiwareMWClientService self);

/**
 * Add plugin.
 * @param[in] self      MW Client Service handle
 * @param[in] plugin    MW Client Service plugin
 */
SvExport void
CubiwareMWClientServiceAddPlugin(CubiwareMWClientService self, SvObject plugin);

/**
 * Remove plugin.
 * @param[in] self      MW Client Service handle
 * @param[in] plugin    MW Client Service plugin
 */
SvExport void
CubiwareMWClientServiceRemovePlugin(CubiwareMWClientService self, SvObject plugin);

/**
 * @}
 **/

#endif // CUBIWAREMWCLIENTSERVICE_H_

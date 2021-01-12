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

#ifndef QBPROVIDERSCCONTROLLERSERVICE_H_
#define QBPROVIDERSCCONTROLLERSERVICE_H_

/**
 * @file QBProvidersControllerService.h Providers controller Service API
 **/

/**
 * @defgroup QBProvidersControllerService Providers Controller Service
 * @ingroup CubiTV_services
 * @{
 **/

#include <main_decl.h>
#include <QBContentManager/Innov8onProvider.h>
#include <Menus/carouselMenu.h>
#include <SvFoundation/SvHashTable.h>

/**
 * @class QBProvidersControllerService QBProvidersControllerService.h <Services/QBProvidersControllerService.h>
 * @extends SvObject
 **/
typedef struct QBProvidersControllerService_t *QBProvidersControllerService;

typedef enum QBProvidersControllerServiceType {
    QBProvidersControllerService_DLNA = 0,
    QBProvidersControllerService_MSTORE,
    QBProvidersControllerService_INNOV8ON,
    QBProvidersControllerService_TRAXIS,
    QBProvidersControllerService_MWClient,
    QBProvidersControllerService_DSMCC,
    QBProvidersControllerService_cnt
} QBProvidersControllerServiceType;

//-------------------------------------------------------------
typedef void
(*QBProvidersControllerServiceServiceAdded)(SvGenericObject self_, SvString serviceId);

typedef void
(*QBProvidersControllerServiceServiceRemoved)(SvGenericObject self_, SvString serviceId);

typedef void
(*QBProvidersControllerServiceServiceModified)(SvGenericObject self_, SvString serviceId);

struct QBProvidersControllerServiceListener_t {
    QBProvidersControllerServiceServiceAdded serviceAdded;
    QBProvidersControllerServiceServiceRemoved serviceRemoved;
    QBProvidersControllerServiceServiceModified serviceModified;
};
typedef struct QBProvidersControllerServiceListener_t* QBProvidersControllerServiceListener;

SvInterface QBProvidersControllerServiceListener_getInterface(void);

void
QBProvidersControllerServiceAddListener(QBProvidersControllerService self,
                                        SvGenericObject listener,
                                        enum QBProvidersControllerServiceType type);

void
QBProvidersControllerServiceRemoveListener(QBProvidersControllerService self,
                                           SvGenericObject listener,
                                           enum QBProvidersControllerServiceType type);

// -----------------------------------------------------------------

/**
 * QBSubmenuFactory interface
 **/
typedef const struct QBSubmenuFactory_ {
    /**
     * Create carousel menu item service.
     *
     * @param[in] self   object implementing QBSubmenuFactory interface
     * @param[in] params submenu parameters
     * @return    handle to new QBCarouselMenuItemService object ord @c NULL in case of error
     **/
    QBCarouselMenuItemService (*createSubmenu)(SvObject self, SvObject params_);
} *QBSubmenuFactory;

/**
 * Get runtime type identification object representing QBSubmenuFactory interface.
 *
 * @return QBSubmenuFactory interface object
 **/
SvInterface
QBSubmenuFactory_getInterface(void);

/**
 * Register object implementing QBSubmenuFactory interface and associate it
 * with service id.
 *
 * @public @memberof QBProvidersControllerService
 *
 * If factory for given id is not found, for backward compatibility, submenu is
 * created in old way.
 *
 * @param[in] self              handle to QBProvidersControllerService
 * @param[in] type              submenu factory service type
 * @param[in] id                identifier
 * @param[in] submenuFactory    object implementing QBSubmenuFactory interface
 **/
void
QBProvidersControllerServiceRegisterSubmenuFactory(QBProvidersControllerService self,
                                                   QBProvidersControllerServiceType type,
                                                   SvString id,
                                                   SvObject submenuFactory);

//-------------------------------------------------------------
typedef enum {
    QBProvidersControllerServiceMonitorId_CubiwareMW = 0
} QBProvidersControllerServiceMonitorId;

//-------------------------------------------------------------

/**
 * Create instance of ProvidersControllerService
 *
 * @param[in] appGlobals appGlobals handle
 * @return Handle of ProvidersControllerService
 */
QBProvidersControllerService QBProvidersControllerServiceCreate(AppGlobals appGlobals);

/**
 * Add provider implementation to ProvidersControllerService
 *
 * @param[in] self QBProvidersControllerService handle
 * @param[in] type provider type
 */
void QBProvidersControllerServiceAddProvider(QBProvidersControllerService self,
                                             QBProvidersControllerServiceType type);

/**
 * Start ProvidersControllerService
 *
 * @param self QBProvidersControllerService handle
 */
void QBProvidersControllerServiceStart(QBProvidersControllerService self);

/**
 * Stop ProvidersControllerService
 *
 * @param self QBProvidersControllerService handle
 */
void QBProvidersControllerServiceStop(QBProvidersControllerService self);

/**
 * Returns an array of services for given tag
 *
 * @param[in] self QBProvidersControllerService handler
 * @param[in] tag  SvString Type of services to retreive
 * @param[in] type service type
 *
 * @return all current Innov8on services of type given through the tag
 */
SvArray
QBProvidersControllerServiceGetServices(QBProvidersControllerService self, SvString tag,
                                        enum QBProvidersControllerServiceType type);

void QBProvidersControllerServiceReinitialize(QBProvidersControllerService self,
                                              SvArray itemList);

//-------------------------------------------------------------

/**
 * API below is for ProviderControllers implementation
 */

typedef void
(*QBProvidersControllerServiceChildStart)(SvObject self_);
typedef void
(*QBProvidersControllerServiceChildStop)(SvObject self_);
typedef void
(*QBProvidersControllerServiceChildAddMenuNode)(SvObject self_);
typedef void
(*QBProvidersControllerServiceChildReinitialize)(SvObject self_, SvArray itemList);
typedef enum QBProvidersControllerServiceType
(*QBProvidersControllerServiceChildType)(SvObject self_);
typedef SvArray
(*QBProvidersControllerServiceChildGetServices)(SvObject self_, SvString tag);
typedef void
(*QBProvidersControllerServiceChildRegisterSubmenuFactory)(SvObject self_, SvString id, SvObject submenuFactory);

typedef struct QBProvidersControllerServiceChild_t {
    QBProvidersControllerServiceChildStart start;
    QBProvidersControllerServiceChildStop stop;
    QBProvidersControllerServiceChildAddMenuNode addNode;
    QBProvidersControllerServiceChildReinitialize reinitialize;
    QBProvidersControllerServiceChildType getType;
    QBProvidersControllerServiceChildGetServices getServices;
    QBProvidersControllerServiceChildRegisterSubmenuFactory registerSubmenuFactory;
} *QBProvidersControllerServiceChild;

SvInterface
QBProvidersControllerServiceChild_getInterface(void);

typedef void
(*QBProvidersControllerServiceObservableAddListener)(SvObject self_, SvGenericObject listener);
typedef void
(*QBProvidersControllerServiceObservableRemoveListener)(SvObject self_, SvGenericObject listener);
typedef void
(*QBProvidersControllerServiceObservableMonitorAddListener)(SvObject self_, SvGenericObject listener);
typedef void
(*QBProvidersControllerServiceObservableMonitorRemoveListener)(SvObject self_, SvGenericObject listener);

typedef struct QBProvidersControllerServiceObservable_t {
    QBProvidersControllerServiceObservableAddListener addListener;
    QBProvidersControllerServiceObservableRemoveListener removeListener;
} *QBProvidersControllerServiceObservable;

SvInterface
QBProvidersControllerServiceObservable_getInterface(void);

typedef struct QBProvidersControllerServiceMountParams_t
{
    QBCarouselMenuItemService carouselMenu;
    SvString menuNodeID;
    SvHashTable mounts;
    SvHashTable services;
    bool isUnfoldable;
    SvObject controller;
    QBActiveTree externalTree;
} *QBProvidersControllerServiceMountParams;

void
QBProvidersControllerServiceMountService(QBProvidersControllerService self,
                                         QBProvidersControllerServiceMountParams params);

void
QBProvidersControllerServiceUnmountService(QBProvidersControllerService self,
                                           QBProvidersControllerServiceMountParams params);

void
QBProvidersControllerServiceDestroyServices(QBProvidersControllerService self,
                                            SvHashTable mounts,
                                            SvHashTable menuNodes,
                                            SvHashTable services);

void
QBProvidersControllerServiceDoMount(AppGlobals appGlobals, QBCarouselMenuItemService carouselMenu,
                                    SvGenericObject path, QBActiveTree externalTree);

void
QBProvidersControllerServiceDoUnmount(AppGlobals appGlobals, QBCarouselMenuItemService carouselMenu);

/**
 * @}
 **/

#endif

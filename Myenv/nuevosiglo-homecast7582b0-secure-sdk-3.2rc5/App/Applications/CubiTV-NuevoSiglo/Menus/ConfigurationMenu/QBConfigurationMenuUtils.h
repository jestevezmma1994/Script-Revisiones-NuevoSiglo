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

#ifndef QB_CONFIGURATION_MENU_UTILS_H_
#define QB_CONFIGURATION_MENU_UTILS_H_

/**
 * @file QBConfigurationMenuUtils.h Configuration menu utilities
 **/

#include <SvCore/SvErrorInfo.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <main_decl.h>

/**
 * @defgroup QBConfigurationMenuUtils Configuration menu utilities
 * @ingroup QBNewConfigurationMenu
 * @{
 */

/**
 * QBConfiguration menu tick state.
 **/
typedef enum QBConfigurationMenuTickState_e {
    QBConfigurationMenuTickState__Hidden,
    QBConfigurationMenuTickState__On,
    QBConfigurationMenuTickState__Off
} QBConfigurationMenuTickState;

/**
 * Check node tick state.
 *
 * @param[in] node          node for which tick state is checked
 * @return                  node's tick state
 **/
QBConfigurationMenuTickState
QBConfigurationMenuCheckTickState(QBActiveTreeNode node);

/**
 * Configuration menu item node apply callback definition.
 *
 * @param[in] self_          owner handle
 * @param[in] node_          node for which access is granted
 * @param[in] nodePath_      path to a node which access is granted
 **/
typedef bool (*QBConfigurationMenuItemNodeApply)(void *self_, SvObject node_, SvObject nodePath_);

/**
 * Configuration menu restricted nodes handler class.
 *
 * @class QBConfigurationMenuRestrictedNodesHandler QBConfigurationMenuUtils.h <Menus/ConfigurationMenu/QBConfigurationMenuUtils.h>
 * @extends SvObject
 **/
typedef struct QBConfigurationMenuRestrictedNodesHandler_ *QBConfigurationMenuRestrictedNodesHandler;

/**
 * Handle item node selection. Show parental control dialog or confirmation dialog if necessary
 *
 * @public @memberof QBConfigurationMenuRestrictedNodesHandler
 *
 * @param[in] self          restricted nodes handler handle
 * @param[in] node          node which should be handled
 * @param[in] path          path to a node which should be handled
 * @return                   was item node selection handled
 **/
bool
QBConfigurationMenuHandleItemNodeSelection(QBConfigurationMenuRestrictedNodesHandler self,
                                           SvObject node,
                                           SvObject path);

/**
 * Create new Network Information menu item controller.
 *
 * @public @memberof QBConfigurationMenuRestrictedNodesHandler
 *
 * @param[in] appGlobals    application globals
 * @param[in] owner         owner of the restricted nodes handler
 * @param[in] applyCallback callback which is called when access to the node is granted
 * @param[in] menu          menu widget handle in which the restricted node is
 * @param[in] tree          tree for which this item choice is created
 * @param[out] *errorOut    error info
 * @return    New Network Information menu item controller or @c NULL in case of error
 **/
QBConfigurationMenuRestrictedNodesHandler
QBConfigurationMenuRestrictedNodesHandlerCreate(AppGlobals appGlobals,
                                                void *owner,
                                                QBConfigurationMenuItemNodeApply applyCallback,
                                                SvWidget menu,
                                                QBActiveTree tree,
                                                SvErrorInfo *errorOut);

// AMERELES Update VoD Services
/**
 * Crea el diálogo para actualizar los servicios VoD
 *
 * @param[in] self    Network Information menu item controller
 **/
void QBConfigurationMenuShowUpdateVODServicesDialog(QBConfigurationMenuRestrictedNodesHandler self);


/**
 * @}
 **/

#endif /* QB_CONFIGURATION_MENU_UTILS_H_ */

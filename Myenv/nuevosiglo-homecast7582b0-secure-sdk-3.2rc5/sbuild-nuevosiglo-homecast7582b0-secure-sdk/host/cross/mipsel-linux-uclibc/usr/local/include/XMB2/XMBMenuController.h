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

#ifndef XMB_MENU_CONTROLLER_H_
#define XMB_MENU_CONTROLLER_H_

/**
 * @file XMBMenuController.h
 * @brief Xross Media Bar menu controller interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup XMBMenuController XMB menu controller interface
 * @ingroup XMB
 * @{
 *
 * An interface for classes that replace default @ref XMBVerticalMenu
 * with their own implementation.
 **/


/**
 * XMB menu controller interface.
 **/
typedef const struct XMBMenuController_t {
    /**
     * Create new XMB submenu widget for given node.
     *
     * @param[in] self_     handle to object implementing item controller
     * @param[in] node      menu node to create submenu for
     * @param[in] path      tree path describing @a node location in the tree
     *                      (@c NULL for standalone XMBMenu widgets)
     * @param[in] app       CUIT application handle
     * @return              handle to the newly created XMBMenu widget,
     *                      @c NULL in case of error
     **/
    SvWidget (*createSubMenu)(SvObject self_,
                              SvObject node,
                              SvObject path,
                              SvApplication app);
} *XMBMenuController;


/**
 * Get runtime type identification object representing
 * XMBMenuController interface.
 *
 * @return XMBMenuController interface object
**/
extern SvInterface
XMBMenuController_getInterface(void);


/**
 * @}
 **/

#endif

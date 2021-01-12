/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_CONTENT_MANAGER_H_
#define QB_CONTENT_MANAGER_H_

/**
 * @file QBContentManager.h
 * @brief Cubiware Content Manager library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBContentManager Content Manager library
 * @ingroup DataLayer
 **/

/**
 * @defgroup QBContentManagerCore Content Manager core
 * @ingroup QBContentManager
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>

#include <QBContentManager/QBContentTree.h>
#include <QBContentManager/QBContentTreePath.h>
#include <QBContentManager/QBContentAction.h>

/**
 * QBContentManager interface
 **/
typedef const struct QBContentManager_ {
    /**
     * create content provider and connect it to the tree at given path
     *
     * @param [in] self_        QBContentManager handle
     * @param [in] tree         data tree to which the provider will be providing data
     * @param [in] path         path in the tree that indicates the node which will recieve data from provider
     * @param [in] service      Object identyfying the type and purpose of the provider that needs to be created
     * @param [out] errorOut    error output.
     * @return                  created content provider or @c NULL in case of error
     **/
    SvObject (*createContentProvider)(SvObject self_,
                                      QBContentTree tree, QBContentTreePath path, SvObject service,
                                      SvErrorInfo *errorOut);
    /**
     * create content provider that will provide action for object returned by other provider
     *
     * @param [in] self_        QBContentManager handle
     * @param [in] tree         data tree which contains the object whose action will be handled by new provider
     * @param [in] provider     parent data provider which provided the object whose action will be handled by new provider
     * @param [in] content      object whose action will be handled by new provider
     * @param [in] action       object representing action that will be handled by the provider
     * @param [out] errorOut    error output.
     * @return                  created action content provider or @c NULL in case of error
     **/
    SvObject (*createActionContentProvider)(SvObject self_,
                                            QBContentTree tree, SvObject provider,
                                            SvObject content, QBContentAction action,
                                            SvErrorInfo *errorOut);

    /**
     * create request object that will handle action for a given object
     *
     * @param [in] self_        QBContentManager handle
     * @param [in] object       object whose action will be handled by request
     * @param [in] action       object representing action that will be handled by the request
     * @param [out] response    array that will have the response objects from the request
     * @param [out] errorOut    error output.
     * @return                  created request object or @c NULL in case of error
     **/
    SvObject (*createActionRequest)(SvObject self_,
                                    SvObject object,
                                    QBContentAction action, SvArray response,
                                    SvErrorInfo *errorOut);

    /**
     * create an array of all possible actions for a given object
     *
     * @param [in] self_        QBContentManager handle
     * @param [in] content      object whose actions will be returned
     * @param [out] errorOut    error output.
     * @return                  array containing all possible actions for content
     **/
    SvArray (*constructActionsForContent)(SvObject self_,
                                          SvObject content,
                                          SvErrorInfo *errorOut);

    /**
     * create service monitor for content manager (see QBContentServiceMonitor)
     *
     * @param [in] self_        QBContentManager handle
     * @param [out] errorOut    error output.
     * @return                  new service monitor object or @c NULL in case of error
     **/
    SvObject (*createServiceMonitor)(SvObject self_,
                                     SvErrorInfo *errorOut);
} *QBContentManager;

/**
 * get the object representing QBContentManager interface
 *
 * @return  object representing runtime type representing QBContentManager interface
 **/
extern SvInterface
QBContentManager_getInterface(void);


#endif

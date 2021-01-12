/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TREE_MODEL_LISTENER_H_
#define QB_TREE_MODEL_LISTENER_H_

/**
 * @file QBTreeModelListener.h Tree data model listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTreeModelListener Tree data model listener interface
 * @ingroup QBDataModel3
 * @{
 *
 * An interface for objects that want to receive notifications
 * about changes in the tree data model.
 **/

/**
 * Tree data model listener interface.
 **/
typedef const struct QBTreeModelListener_ {
    /**
     * Notify that new tree nodes appeared in the data source.
     *
     * This method notifies the listener that some number of new
     * nodes appeared as child nodes of the tree node identified
     * by @a path.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreeModelListener
     * @param[in] path      path of the parent node of newly added nodes
     * @param[in] first     index of the first added node
     * @param[in] count     number of added nodes
     **/
    void (*nodesAdded)(SvObject self_,
                       SvObject path,
                       size_t first,
                       size_t count);

    /**
     * Notify that some tree nodes disappeared from the data source.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreeModelListener
     * @param[in] path      path of the parent node of removed nodes
     * @param[in] first     index of the first removed node
     * @param[in] count     number of removed nodes
     **/
    void (*nodesRemoved)(SvObject self_,
                         SvObject path,
                         size_t first,
                         size_t count);

    /**
     * Notify that some nodes changed.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreeModelListener
     * @param[in] path      path of the parent node of changed nodes
     * @param[in] first     index of the first modified node
     * @param[in] count     number of modified nodes
     **/
    void (*nodesChanged)(SvObject self_,
                         SvObject path,
                         size_t first,
                         size_t count);
} *QBTreeModelListener;
#define QBTreeModelListener_t QBTreeModelListener_


/**
 * Get runtime type identification object representing
 * QBTreeModelListener interface.
 *
 * @return QBTreeModelListener interface object
 **/
extern SvInterface
QBTreeModelListener_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

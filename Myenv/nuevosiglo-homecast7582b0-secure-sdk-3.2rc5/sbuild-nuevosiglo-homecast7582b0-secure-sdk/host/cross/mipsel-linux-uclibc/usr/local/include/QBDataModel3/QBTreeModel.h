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

#ifndef QB_TREE_MODEL_H_
#define QB_TREE_MODEL_H_

/**
 * @file QBTreeModel.h
 * @brief Tree data model interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBTreeIterator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTreeModel Tree data model interface
 * @ingroup QBDataModel3
 * @{
 *
 * Tree data model is an abstract interface for any source of data that
 * can be thought of as dynamically changing tree of objects.
 *
 * @image html QBTreeModel.png
 **/

/**
 * Tree data model interface.
 **/
typedef const struct QBTreeModel_ {
    /**
     * Get node identified by given tree path.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreeModel
     * @param[in] path      handle to a tree path, @c NULL for root node
     * @return              tree node handle, @c NULL if not found
     **/
    SvObject (*getNode)(SvObject self_,
                        SvObject path);

    /**
     * Get iterator over child nodes of a node identified by given tree path.
     *
     * @note If @a position is invalid (there is no child node at this position),
     *       returned value is undefined. You must check beforehand if @a position
     *       is valid using methods provided by tree model implementation or
     *       QBTreeIteratorGetNodesCount().
     *
     * @warning Tree iterator is not an object, so it doesn't have a destructor
     *          and is not able to retain/release provided @a path. It is
     *          caller's responsibility to guarantee that @a path is valid
     *          and is not modified for as long as iterator is used.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreeModel
     * @param[in] path      handle to a tree path, @c NULL for root node
     * @param[in] position  initial position of the iterator (index of the
     *                      node under @a path the iterator will point to)
     * @return              tree iterator
     **/
    QBTreeIterator (*getIterator)(SvObject self_,
                                  SvObject path,
                                  size_t position);
} *QBTreeModel;
#define QBTreeModel_t QBTreeModel_


/**
 * Get runtime type identification object representing
 * QBTreeModel interface.
 *
 * @return QBTreeModel interface object
 **/
extern SvInterface
QBTreeModel_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

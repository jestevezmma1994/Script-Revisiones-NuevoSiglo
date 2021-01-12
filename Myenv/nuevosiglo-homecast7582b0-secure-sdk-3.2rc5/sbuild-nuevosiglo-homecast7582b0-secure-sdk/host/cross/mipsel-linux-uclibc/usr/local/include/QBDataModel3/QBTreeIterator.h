/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TREE_ITERATOR_H_
#define QB_TREE_ITERATOR_H_

/**
 * @file QBTreeIterator.h Tree iterator type
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTreeIterator Tree iterator type
 * @ingroup QBDataModel3
 * @{
 *
 * An iterator for traversing a tree.
 **/

/**
 * Tree iterator type.
 **/
typedef struct QBTreeIterator_ QBTreeIterator;


/**
 * QBTreeIterator methods.
 * @internal
 **/
struct QBTreeIteratorMethods {
    /**
     * Get number of nodes that can be fetched by the iterator.
     *
     * @param[in] subject   an object iterated over
     * @param[in] iterator  iterator handle
     * @return              number of nodes available
     **/
    size_t (*getNodesCount)(SvObject subject,
                            const QBTreeIterator *iterator);

    /**
     * Get node pointed to by the iterator and advance iterator's position.
     *
     * @param[in] subject   an object iterated over
     * @param[in,out] iterator iterator handle
     * @return              a handle to the next node, @c NULL if unavailable
     **/
    SvObject (*getNextNode)(SvObject subject,
                            QBTreeIterator *iterator);

    /**
     * Get path to current node.
     *
     * @param[in] subject   an object iterated over
     * @param[in] iterator  iterator handle
     * @return              path to the current node or @c NULL
     **/
    SvObject (*getCurrentNodePath)(SvObject subject,
                                   const QBTreeIterator *iterator);
};
#define QBTreeIterable_t QBTreeIteratorMethods


/**
 * QBTreeIterator type internals.
 * @internal
 **/
struct QBTreeIterator_ {
    /// an object iterated over
    SvObject subject;
    /// iterator implementation
    const struct QBTreeIteratorMethods *methods;
    /// path to the next tree node
    SvObject path;
    union {
        /// value as index or size
        size_t z;
        /// value as a pointer
        void *ptr;
    } auxA, ///< current position/value of the iterator, first part
      auxB; ///< current position/value of the iterator, second part
};


/**
 * Get number of nodes that can be fetched by the iterator.
 *
 * @param[in] iterator  tree iterator handle
 * @return              number of nodes available
 **/
static inline size_t
QBTreeIteratorGetNodesCount(const QBTreeIterator *iterator)
{
    return iterator->methods->getNodesCount(iterator->subject, iterator);
}

/**
 * Get node pointed to by the iterator and advance iterator's position.
 *
 * @param[in,out] iterator iterator handle
 * @return              a handle to the next node, @c NULL if unavailable
 **/
static inline SvObject
QBTreeIteratorGetNextNode(QBTreeIterator *iterator)
{
    return iterator->methods->getNextNode(iterator->subject, iterator);
}

/**
 * Get path to current node.
 *
 * @param[in] iterator  iterator handle
 * @return              path to current node or @c NULL
 **/
static inline SvObject
QBTreeIteratorGetCurrentNodePath(const QBTreeIterator *iterator)
{
    return iterator->methods->getCurrentNodePath(iterator->subject, iterator);
}


/**
 * Get a tree iterator that will never point to any object.
 * @return void tree iterator
 **/
extern QBTreeIterator
QBGetVoidTreeIterator(void);

/** @cond */
static inline QBTreeIterator
QBVoidTreeIterator(void)
{
    return QBGetVoidTreeIterator();
}
/** @endcond */

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

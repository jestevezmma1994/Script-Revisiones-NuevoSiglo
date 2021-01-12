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

#ifndef SvFoundation_SvBinaryTree_h
#define SvFoundation_SvBinaryTree_h

/**
 * @file SvBinaryTree.h Binary tree class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvComparator.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvBinaryTree Binary tree class
 * @ingroup SvFoundationCollections
 * @{
 **/


/**
 * Binary AVL tree class.
 * @class SvBinaryTree
 * @extends SvObject
 **/
typedef struct SvBinaryTree_ *SvBinaryTree;


/**
 * Get runtime type identification object representing SvBinaryTree class.
 *
 * @return binary tree class
 **/
extern SvType
SvBinaryTree_getType(void);


/**
 * Initialize allocated SvBinaryTree instance with a comparator object.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] comparator a handle to an object implementing @ref SvComparator
 * @param[out] errorOut error info
 * @return @a self or @c NULL in case of error
 **/
extern SvBinaryTree
SvBinaryTreeInit(SvBinaryTree self,
                 SvObject comparator,
                 SvErrorInfo *errorOut);

/**
 * Initialize allocated SvBinaryTree instance with a comparing function.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] compare function for comparing objects
 * @param[in] prv opaque pointer to @a compare function's private data
 * @param[out] errorOut error info
 * @return @a self or @c NULL in case of error
 **/
extern SvBinaryTree
SvBinaryTreeInitWithCompareFn(SvBinaryTree self,
                              SvObjectCompareFn compare,
                              void *prv,
                              SvErrorInfo *errorOut);

/**
 * Create SvBinaryTree object with a comparator object.
 *
 * This is a convenience method that allocates and initializes
 * SvBinaryTree object.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] comparator a handle to an object implementing @ref SvComparator
 * @param[out] errorOut error info
 * @return @a self or @c NULL in case of error
 **/
extern SvBinaryTree
SvBinaryTreeCreate(SvObject comparator,
                   SvErrorInfo *errorOut);

/**
 * Create SvBinaryTree object with a comparing function.
 *
 * This is a convenience method that allocates and initializes
 * SvBinaryTree object.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] compare function for comparing objects
 * @param[in] prv opaque pointer to @a compare function's private data
 * @param[out] errorOut error info
 * @return @a self or @c NULL in case of error
 **/
extern SvBinaryTree
SvBinaryTreeCreateWithCompareFn(SvObjectCompareFn compare,
                                void *prv,
                                SvErrorInfo *errorOut);

/**
 * Get number of nodes (objects) in a tree.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @return number of nodes in a tree, (size_t) -1 in case of error
 **/
extern size_t
SvBinaryTreeGetNodesCount(SvBinaryTree self);

/** @cond */
static inline size_t
SvBinaryTreeNodesCount(SvBinaryTree self)
{
    return SvBinaryTreeGetNodesCount(self);
}
/** @endcond */

/**
 * Get an object stored in the tree at @a index
 * when tree is traversed "inorder".
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] idx index of an object to be returned
 * @return object at @a idx or @c NULL in case of error
 **/
extern SvObject
SvBinaryTreeGetObjectAtIndex(SvBinaryTree self,
                             size_t idx);

/** @cond */
static inline SvObject
SvBinaryTreeObjectAtIndex(SvBinaryTree self,
                          size_t idx)
{
    return SvBinaryTreeGetObjectAtIndex(self, idx);
}
/** @endcond */

/**
 * Get an object stored in the tree identical to @a object.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] object object key
 * @return object identical to @a object or @c NULL if not found
 **/
extern SvObject
SvBinaryTreeGetObject(SvBinaryTree self,
                      SvObject object);

/**
 * Get an index of the given @a object in the tree
 * traversed "inorder".
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] value an object to be found in the tree
 * @return index of the @a object in the tree traversed "inorder",
 *         @c -1 if not found
 **/
extern ssize_t
SvBinaryTreeGetObjectIndex(SvBinaryTree self,
                           SvObject value);

/**
 * Create an array with all objects in the tree.
 *
 * @memberof SvBinaryTree
 * @qb_allocator
 *
 * @param[in] self binary tree handle
 * @param[out] errorOut error info
 * @return array of all objects in the tree
 **/
extern SvArray
SvBinaryTreeCreateElementsList(SvBinaryTree self,
                               SvErrorInfo *errorOut);

/**
 * Insert new object to the binary tree.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] value an object to be added to the tree
 **/
extern void
SvBinaryTreeInsert(SvBinaryTree self,
                   SvObject value);

/**
 * Remove object from the binary tree.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] value an object to be removed from the tree
 **/
extern void
SvBinaryTreeRemove(SvBinaryTree self,
                   SvObject value);

/**
 * Remove all objects from the binary tree.
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 **/
extern void
SvBinaryTreeRemoveAllObjects(SvBinaryTree self);

/**
 * Find position of first object that's greater to given @a key.
 *
 * This method returns an iterator matching following criteria:
 *   *--iter <= key < *iter
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] key key to compare binary tree contents to
 * @return iterator
 **/
extern SvIterator
SvBinaryTreeGetUpperBound(SvBinaryTree self,
                          SvObject key);

/**
 * Find position of first object that's greater or equal to given @a key.
 *
 * This method returns an iterator matching following criteria:
 * *--iter < key <= *iter
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @param[in] key key to compare binary tree contents to
 * @return iterator
 **/
extern SvIterator
SvBinaryTreeGetLowerBound(SvBinaryTree self,
                          SvObject key);

/** @cond */
static inline SvIterator
SvBinaryTreeLowerBound(SvBinaryTree self,
                       SvObject key)
{
    return SvBinaryTreeGetLowerBound(self, key);
}
/** @endcond */

/**
 * Get an iterator for iterating over a binary tree
 * (traversing a tree "inorder".)
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @return the iterator
 **/
extern SvIterator
SvBinaryTreeGetIterator(SvBinaryTree self);

/** @cond */
static inline SvIterator
SvBinaryTreeIterator(SvBinaryTree self)
{
    return SvBinaryTreeGetIterator(self);
}
/** @endcond */

/**
 * Get an iterator for iterating over a binary tree in reverse order
 * (traversing a tree in the direction opposite to "inorder".)
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @return the reverse iterator
 **/
extern SvIterator
SvBinaryTreeGetReverseIterator(SvBinaryTree self);

/**
 * Get an iterator for traversing a binary tree "preorder".
 *
 * @memberof SvBinaryTree
 *
 * @param[in] self binary tree handle
 * @return the preorder iterator
 **/
extern SvIterator
SvBinaryTreeGetPreOrderIterator(SvBinaryTree self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

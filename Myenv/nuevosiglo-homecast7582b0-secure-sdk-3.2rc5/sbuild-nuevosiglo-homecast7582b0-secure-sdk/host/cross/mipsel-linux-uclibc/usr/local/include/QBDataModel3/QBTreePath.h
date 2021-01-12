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

#ifndef QB_TREE_PATH_H_
#define QB_TREE_PATH_H_

/**
 * @file QBTreePath.h Tree path interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTreePath Tree path interface
 * @ingroup QBDataModel3
 * @{
 *
 * An interface for objects representing path from root node
 * to other node of a tree.
 **/

/**
 * Tree path interface.
 **/
typedef const struct QBTreePath_ {
    /**
     * Get number of path components.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreePath
     * @return              number of path components
     **/
    size_t (*getLength)(SvObject self_);

    /**
     * Truncate path.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreePath
     * @param[in] n         if @a n >= @c 0: desired path length,
     *                      otherwise: relative path length change
     * @return              number of path components after truncation
     **/
    size_t (*truncate)(SvObject self_,
                       ssize_t n);

    /**
     * Get subpath of a tree path.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreePath
     * @return              subpath handle, can be @c NULL if not set
     **/
    SvObject (*getSubPath)(SvObject self_);

    /**
     * Set subpath of a tree path.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreePath
     * @param[in] subPath   subpath handle, @c NULL to remove current subpath
     **/
    void (*setSubPath)(SvObject self_,
                       SvObject subPath);

    /**
     * Calculate length of the common part of two paths.
     *
     * @param[in] self_     handle to an object implementing @ref QBTreePath
     * @param[in] other     handle to an object implementing @ref QBTreePath
     * @return              length of common part
     **/
    size_t (*getCommonPartLength)(SvObject self_,
                                  SvObject other);
} *QBTreePath;
#define QBTreePath_t QBTreePath_


/**
 * Get runtime type identification object representing
 * QBTreePath interface.
 *
 * @return QBTreePath interface object
 **/
extern SvInterface
QBTreePath_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

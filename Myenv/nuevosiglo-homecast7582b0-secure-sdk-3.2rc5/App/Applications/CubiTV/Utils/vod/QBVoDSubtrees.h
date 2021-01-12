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

#ifndef QB_VOD_SUBTREES_H
#define QB_VOD_SUBTREES_H

/**
 * @file QBVoDSubtrees.h Subtrees container class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvArray.h>

/**
 * @defgroup QBVoDSubtrees Subtrees container class
 * @ingroup CubiTV_utils
 * @{
 **/

/**
 * QBVoDSubtrees class is a container used to manage and hold reference to
 * QBContentTree objects used in VoD view alongside the main VoD data tree.
 * Some vod views may use trees of data
 * other than a main VoD tree (ie. tree for recommendations).
 * If those trees are stored somewhere then there is no need to
 * recreate them every time a user want's to display data from
 * that tree, as the trees from container can be reused.
 * @class QBVoDSubtrees
 * @extends SvObject
 **/
typedef struct QBVoDSubtrees_* QBVoDSubtrees;

/**
 * QBVoDSubtree class is used to hold the reference to a data tree
 * and some data associated with that tree (ie a data providers).
 **/
typedef struct QBVoDSubtree_* QBVoDSubtree;

/**
 * Get runtime type of QBVoDSubtrees class.

 *
 * @return  Type representing QBVoDSubtrees
 **/
SvType
QBVoDSubtrees_getType(void);

/**
 * Create subtrees instance.
 *
 * @return  new instance of QBVoDSubtrees
 **/
QBVoDSubtrees
QBVoDSubtreesCreate(void);

/**
 * Get tree stored under an index.
 *
 * @param [in] self     subtrees reference
 * @param [in] id       id of tree to get
 * @return              new tree stored under given index, or @c NULL if not present
 **/
QBVoDSubtree
QBVoDSubtreesGetSubtree(QBVoDSubtrees self, int id);

/**
 * Set tree stored under an index.
 *
 * @param [in] self     subtrees reference
 * @param [in] id       id of tree to get
 * @param [in] subtree  tree to store in the container
 **/
void
QBVoDSubtreesSetSubtree(QBVoDSubtrees self, int id, QBVoDSubtree subtree);

/**
 * Create an object holding reference to a tree.
 *
 * @param [in] tree     tree object
 * @return              new instance of QBVoDSubtree
 **/
QBVoDSubtree
QBVoDSubtreeCreate(SvObject tree);

/**
 * Add data provider associated with a tree.
 *
 * @param [in] self     this object
 * @param [in] provider provider to be associated with a tree.
 **/
void
QBVoDSubtreeAddProvider(QBVoDSubtree self, SvObject provider);

/**
 * Stop providers associated with a tree.
 *
 * @param [in] self     this object
 **/
void
QBVoDSubtreeStopProviders(QBVoDSubtree self);

/**
 * Get tree object stored in QBVoDSubtree
 *
 * @param [in] self     this object
 * @return              tree object stored in QBVoDSubtree
 **/
SvObject
QBVodSubtreeGetTree(QBVoDSubtree self);

/**
 * Get the number of providers associated with this tree
 *
 * @param [in] self     this object
 * @return              number of providers associated with a tree
 **/
size_t
QBVodSubtreeGetProvidersCount(QBVoDSubtree self);

/**
 * Get the provider associated with a tree stored at given index
 *
 * @param [in] self     this object
 * @param [in] index    index of provider
 * @return              Provider or @c NULL if index is invalid.
 **/
SvObject
QBVodSubtreeGetProvider(QBVoDSubtree self, size_t index);

/**
 * Set additional data associated with a tree.
 *
 * @param [in] self     this object
 * @param [in] key      key under which the data should be storred
 * @param [in] data     data to be storred
 **/
void
QBVodSubtreeSetData(QBVoDSubtree self, SvString key, SvObject data);

/**
 * Get additional data associated with a tree.
 *
 * @param [in] self     this object
 * @param [in] key      key under which to find the data
 * @return              data storred under key or @c NULL if no data found under key
 **/
SvObject
QBVodSubtreeGetData(QBVoDSubtree self, SvString key);

/**
 * @}
 **/

#endif

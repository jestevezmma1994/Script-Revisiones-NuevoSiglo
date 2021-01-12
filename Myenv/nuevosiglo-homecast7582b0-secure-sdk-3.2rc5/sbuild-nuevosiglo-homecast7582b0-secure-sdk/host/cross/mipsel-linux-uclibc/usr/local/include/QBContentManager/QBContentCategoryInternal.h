/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CONTENT_CATEGORY_INTERNAL_H_
#define QB_CONTENT_CATEGORY_INTERNAL_H_


#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentTree.h>
#include <QBAppKit/QBObservable.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvFoundation/SvWeakList.h>
#include <SvCore/SvErrorInfo.h>

#include <stddef.h>


struct QBContentCategory_ {
    /// super class
    struct QBObservable_ super_;

    /// current loading state
    QBContentCategoryLoadingState loadingState;
    /// category ID
    SvString id;
    /// category name
    SvString name;
    /// category tile
    SvString title;
    /// distance from root category
    int level;
    /// custom attributes, see QBContentCategoryGetAttribute()
    SvHashTable attributes;

    /// provider reference type
    enum {
        ProviderReferenceType_weak,
        ProviderReferenceType_hard,
    } providerReferenceType;

    /// VoD provider that populates this category
    union {
        SvObject reference;
        SvWeakReference weakReference;
        SvObject hardReference;
    } provider;

    /// VoD tree in which this category resides
    SvWeakReference tree;

    /// parent category, @c NULL in root category
    QBContentCategory parent;

    /// scheduler used for batch tasks
    SvScheduler scheduler;

    /// static items shown before dynamic content
    SvArray staticItems;
    /// number of static items
    size_t staticCount;

    /// child categories
    SvHashTable subcategories;

    /// dynamic items collection; implements @ref QBSearchableListModel
    SvObject items;

    /// listeners
    SvWeakList listeners;
};


extern QBContentCategory QBContentCategoryFindSubcategory(QBContentCategory self, SvString name);

extern int QBContentCategoryRemoveStaticItem(QBContentCategory self, SvObject item);

extern bool QBContentCategoryContainsObject(QBContentCategory self, SvObject object);

extern void QBContentCategoryPropagateStaticItemsChange(QBContentCategory self);


/**
 * Initializes a instance of content category.
 *
 * @param[in] self      content category handle
 * @param[in] id        id of the category, must be unique in the tree
 * @param[in] name      deprecated, means the same as id
 * @param[in] title     name of the category, it must be user-displayable as
                        title is set to this value.
 * @param[in] parent    handle to parent category, NULL if the category should be root
 * @param[in] tree      handle to the tree in which the category will be available
 * @param[out] errorOut error info
 * @return returns self
 **/
extern QBContentCategory
QBContentCategoryInit(QBContentCategory self,
                      SvString id,
                      SvString name,
                      SvString title,
                      QBContentCategory parent,
                      SvObject tree,
                      SvErrorInfo *errorOut);

/**
 * Initializes a instance of content category.
 *
 * @param[in] self        content category handle
 * @param[in] id          id of the category, must be unique in the tree
 * @param[in] title       name of the category, it must be user-displayable as
                          title is set to this value.
 * @param[in] provider    provider, will be retained by hard reference
 * @param[in] itemsSource items source (implementing QBDataModel and QBSearchableListModel)
 * @param[out] errorOut   error info
 * @return returns self
 **/
extern QBContentCategory
QBContentCategoryInitWithProviderHardReferenceAndItemsSource(QBContentCategory self,
                                                             SvString id,
                                                             SvString title,
                                                             SvObject provider,
                                                             SvObject itemsSource,
                                                             SvErrorInfo *errorOut);
/**
 * Set tree for a content category.
 *
 * This method should be used only by QBContentTree when it's created with given category.
 *
 * @param[in] self       content category handle
 * @param[in] tree       handle to the tree in which the category will be available
 * @param[out] errorOut  error info
 * @return returns self
 **/
extern void
QBContentCategorySetTree(QBContentCategory self,
                         QBContentTree tree,
                         SvErrorInfo *errorOut);

#endif

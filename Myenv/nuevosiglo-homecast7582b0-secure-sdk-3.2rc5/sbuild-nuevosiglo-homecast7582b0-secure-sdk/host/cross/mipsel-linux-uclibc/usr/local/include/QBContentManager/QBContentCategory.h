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

#ifndef QB_CONTENT_CATEGORY_H_
#define QB_CONTENT_CATEGORY_H_

/**
 * @file QBContentCategory.h
 * @brief Content category class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <QBContentManager/QBContentCommonDefs.h>


/**
 * @defgroup QBContentCategory Content category class
 * @ingroup QBContentManagerCore
 * @{
 **/

/**
 * Loading state of a content category.
 **/
typedef enum {
    /// unknown state marker
    QBContentCategoryLoadingState_unknown = -1,
    /// initial state, when number of items is yet unknown
    QBContentCategoryLoadingState_initial = 0,
    /// active state, when data fetch is in progress
    QBContentCategoryLoadingState_active,
    /// idle state, nothing happens in the background
    QBContentCategoryLoadingState_idle,
} QBContentCategoryLoadingState;

/**
 * Content category class.
 *
 * It is an object representing a collection of sub-objects provided by a Content Provider.
 * Provides an data source of sub-objects that implements a QBListModel.
 **/
typedef struct QBContentCategory_ *QBContentCategory;


/**
 * Get runtime type identification object representing
 * type of content category class.
 * @return content category runtime type identification object
 **/
extern SvType
QBContentCategory_getType(void);

/**
 * Creates a new category.
 *
 * @param[in] id        id of the category, must be unique in the tree
 * @param[in] name      name of the category, it must be user-displayable as
                        title is set to this value.
 * @param[in] parent    handle to parent category, NULL if the category should be root
 * @param[in] tree      handle to the tree in which the category will be available
 * @param[out] errorOut error info
 * @return new content category instance
 **/
extern QBContentCategory
QBContentCategoryCreate(SvString id,
                        SvString name,
                        QBContentCategory parent,
                        SvObject tree,
                        SvErrorInfo *errorOut);

/**
 * Get content category name.
 *
 * @param[in] self      content category handle
 * @return              category name, @c NULL in case of error
 **/
extern SvString
QBContentCategoryGetName(QBContentCategory self);

/**
 * Change content category name.
 *
 * @param[in] self      content category handle
 * @param[in] name     new name
 * @param[out] errorOut error info
 **/
void
QBContentCategorySetName(QBContentCategory self,
                         SvString name,
                         SvErrorInfo *errorOut);

/**
 * Get content category title to be displayed. In most cases name and title are equal.
 *
 * @param[in] self      content category handle
 * @return              category title, @c NULL in case of error
 */
extern SvString
QBContentCategoryGetTitle(QBContentCategory self);

/**
 * Change content category title.
 *
 * @param[in] self      content category handle
 * @param[in] title     new title, @c NULL to remove (category name will be used instead)
 * @param[out] errorOut error info
 **/
extern void
QBContentCategorySetTitle(QBContentCategory self,
                          SvString title,
                          SvErrorInfo *errorOut);

/**
 * Get content category unique ID.
 *
 * @param[in] self      content category handle
 * @return              category ID, @c NULL in case of error
 */
extern SvString
QBContentCategoryGetId(QBContentCategory self);

/**
 * Get parent content category.
 *
 * @param[in] self      content category handle
 * @return              parent category handle, @c NULL if unavailable
 **/
extern QBContentCategory
QBContentCategoryGetParent(QBContentCategory self);

/**
 * Get loading state of a content category.
 *
 * @param[in] self      content category handle
 * @return              loading state
 **/
extern QBContentCategoryLoadingState
QBContentCategoryGetLoadingState(QBContentCategory self);

/**
 * Set loading state of a content category.
 *
 * @param[in] self      content category handle
 * @param[in] state     new loading state
 * @param[out] errorOut error info
 **/
extern void
QBContentCategorySetLoadingState(QBContentCategory self,
                                 QBContentCategoryLoadingState state,
                                 SvErrorInfo *errorOut);

/**
 * Get content category level (distance from root category of selected category).
 *
 * @param[in] self      content category handle
 * @return              category level  (distance from root category of selected category)
 */
extern int
QBContentCategoryGetLevel(QBContentCategory self);

/**
 * Register content category listener.
 *
 * @param[in] self      content category handle
 * @param[in] listener  handle to a @ref QBContentCategoryListener
 * @param[out] errorOut error info
 **/
extern void
QBContentCategoryAddListener(QBContentCategory self,
                             SvObject listener,
                             SvErrorInfo *errorOut);

/**
 * Unregister content category listener.
 *
 * @param[in] self      content category handle
 * @param[in] listener  handle to a previously registered listener
 * @param[out] errorOut error info
 **/
extern void
QBContentCategoryRemoveListener(QBContentCategory self,
                                SvObject listener,
                                SvErrorInfo *errorOut);

/**
 * Sets the data source for children of this category. It must be
 * an object implementing QBSearchableListModel.
 *
 * @param[in] self      content category handle
 * @param[in] source    data source of children
 **/
extern void
QBContentCategorySetItemsSource(QBContentCategory self,
                                SvObject source);

/**
 * Returns the data source for children of this category.
 * It will be an object implementing QBListModel.
 *
 * @param[in] self      content category handle
 * @returns children data source
 **/
extern SvObject
QBContentCategoryGetItemsSource(QBContentCategory self);

/**
 * Informs the content category in which Content Provider it was created.
 * The provider is kept by Weak Reference.
 *
 * @param[in] self      content category handle
 * @param[in] provider  provider handle
 **/
extern void
QBContentCategorySetProvider(QBContentCategory self,
                             SvObject provider);

/**
 * Returns handle to Content Provider which provides this category.
 * Returns strong reference. Caller is responsible for releasing returned object.
 * @param[in] self      content category handle
 * @return strong reference to Content Provider (must be released)
 */
extern SvObject
QBContentCategoryTakeProvider(QBContentCategory self);

/**
 * Sets a custom attribute.
 *
 * It may be used to provide additional, provider-specific metadata
 * about a category.
 *
 * Internally attributes are treated as a hash-table.
 *
 * @param[in] self      content category handle
 * @param[in] name      attribute name (key)
 * @param[in] value     attribute value (value)
 **/
extern void
QBContentCategorySetAttribute(QBContentCategory self,
                              SvString name,
                              SvObject value);

/**
 * Get value of a custom attribute.
 *
 * @param[in] self      content category handle
 * @param[in] name      name of the custom attribute
 * @return              attribute value, @c NULL if unavailable
 **/
extern SvObject
QBContentCategoryGetAttribute(QBContentCategory self,
                              SvString name);

/**
 * Returns the total count of children (static + dynamic).
 *
 * @param[in] self      content category handle
 * @return              amount of children
 **/
extern size_t
QBContentCategoryGetLength(QBContentCategory self);

/**
 * Returns count of static children.
 * @param[in] self      content category handle
 * @return              amount of static children
 **/
extern size_t
QBContentCategoryGetStaticCount(QBContentCategory self);


/**
 * Removes a subcategory identified by id. Notifications will not be sent.
 * Only for internal use.
 *
 * @param[in] self      content category handle
 * @param[in] name      id of the category to be removed.
 **/
extern void
QBContentCategoryRemoveSubcategory(QBContentCategory self,
                                   SvString name);

/**
 * Get child of this category at specified index.
 *
 * @param[in] self      content category handle
 * @param[in] index     index of a child to get.
 * @return              child handle or @c NULL if unavailable.
 **/
extern SvObject
QBContentCategoryGetObjectAtIndex(QBContentCategory self, size_t index);

/**
 * Add static item at the beggining of the list of children of this category.
 *
 * @param[in] self      content category handle
 * @param[in] item      Static item to be prepended to the category children.
 **/
extern void QBContentCategoryPrependStaticItem(QBContentCategory self, SvObject item);

/**
 * Request refresh of category data.
 *
 * @param[in] self      content category handle
 * @param[in] index     index at which to refresh data
 * @param[in] force     use @c true to force the provider to perform the refresh
 **/
extern void QBContentCategoryRefresh(QBContentCategory self, size_t index, bool force);

/**
 * Add static item to the category.
 *
 * @param[in] self      content category handle
 * @param[in] item      item to add to category
 **/
extern void QBContentCategoryAddStaticItem(QBContentCategory self, SvObject item);

/**
 * Get the retained pointer to the tree this category belongs to.
 *
 * @param[in] self      content category handle
 * @return              retained pointer to the tree, or @c NULL if this category does not belong to a tree.
 **/
extern QBContentTree QBContentCategoryTakeTree(QBContentCategory self);

/**
 * @}
 **/


#endif

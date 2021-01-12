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

/* App/Libraries/QBContentManager/QBContentTree.h */

#ifndef QB_CONTENT_TREE_H_
#define QB_CONTENT_TREE_H_

/**
 * @file QBContentTree.h
 * @brief Content tree class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBContentTree Content tree class
 * @ingroup QBContentManagerCore
 * @{
 **/

#include <stdlib.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentCommonDefs.h>

/**
 * Get runtime type identification object representing
 * type of content tree class.
 * @return content tree runtime type identification object
 **/
extern SvType
QBContentTree_getType(void);

/**
 * Create new empty content tree.
 *
 * @param[in] rootName  root category name
 * @param[out] errorOut error info
 * @return              created content tree, @c NULL in case of error
 **/
extern QBContentTree
QBContentTreeCreate(SvString rootName,
                    SvErrorInfo *errorOut);

/**
 * Create new content tree with given root category.
 *
 * @param[in] root      root
 * @param[out] errorOut error info
 * @return              created content tree, @c NULL in case of error
 **/
extern QBContentTree
QBContentTreeCreateWithGivenCategory(QBContentCategory root,
                                     SvErrorInfo *errorOut);

/**
 * Add static category to be the new root for content (returned by @c QBContentTreeGetRootCategory).
 * The category created by this function will be appended as child of
 * current content root and then become new content root.
 *
 * @param [in] self             this object
 * @param [in] contentRootName  name of the new content root category
 * @param [out] errorOut        error output or @c NULL if errors are not important.
 **/
extern void
QBContentTreeAddContentRootCategory(QBContentTree self,
                                    SvString contentRootName,
                                    SvErrorInfo *errorOut);

/**
 * Search for category with give id.
 *
 * @param[in] self      content tree handle
 * @param[in] id        category id
 * @param[out] errorOut error info
 * @return              content category, @c NULL in case of error
 **/
SvObject
QBContentTreeFindCategoryById(QBContentTree self,
                              SvString id,
                              SvErrorInfo *errorOut);

/**
 * Content tree will now notify that all static items have changed useing
 * standard tree model interfaces.
 *
 * @param[in] self      content tree handle
 * @param[out] errorOut error info
 **/
void
QBContentTreePropagateStaticItemsChange(QBContentTree self,
        SvErrorInfo *errorOut);

/**
 * Get root category of the content tree.
 *
 * @param[in] self      content tree handle
 * @return              root category of the content tree
 **/
extern QBContentCategory
QBContentTreeGetRootCategory(QBContentTree self);

/**
 * @}
 **/

#endif

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

#include <Utils/contentCategoryUtils.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <SvFoundation/SvInterface.h>

int
QBContentCategoryUtilsGetProductIdxInCategory(QBContentCategory category, SvObject product)
{
    SvObject itemsSource = QBContentCategoryGetItemsSource(category);
    size_t len = SvInvokeInterface(QBListModel, itemsSource, getLength);
    SvObject object = NULL;
    size_t i = 0;
    for (i = 0; i < len; i++) {
        object = SvInvokeInterface(QBListModel, itemsSource, getObject, i);
        if (SvObjectEquals(object, product))
            break;
    }
    return i == len ? -1 : (int) i;
}

QBContentCategory
QBContentCategoryUtilsFindCategory(SvObject tree, SvObject path)
{
    SvObject node = SvInvokeInterface(QBTreeModel, tree, getNode, path);

    if (!node)
        return NULL;

    if (!SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
        SvObject nodePath = SvObjectCopy(path, NULL);
        SvInvokeInterface(QBTreePath, nodePath, truncate, -1);
        node = SvInvokeInterface(QBTreeModel, tree, getNode, nodePath);
        SVRELEASE(nodePath);
        if (!SvObjectIsInstanceOf(node, QBContentCategory_getType()))
            return NULL;
    }
    return (QBContentCategory) node;
}

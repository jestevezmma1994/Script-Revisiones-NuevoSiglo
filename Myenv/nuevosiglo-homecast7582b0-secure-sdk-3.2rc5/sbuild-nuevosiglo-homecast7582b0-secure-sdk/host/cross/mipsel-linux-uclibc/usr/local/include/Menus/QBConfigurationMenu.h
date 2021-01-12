/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCONFIGURATIONMENU_H
#define QBCONFIGURATIONMENU_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <CUIT/Core/types.h>
#include <main_decl.h>

typedef void (*QBConfigurationMenuParentalControlDialogRequested)(SvGenericObject self_, QBActiveTreeNode node, SvGenericObject nodePath);
typedef void (*QBConfigurationMenuHandleAction)(SvGenericObject self_, SvString action);

struct QBConfigurationMenuCallbacks_s {
    QBConfigurationMenuParentalControlDialogRequested authenticationRequested;
    QBConfigurationMenuHandleAction handleAction;
};

typedef struct QBConfigurationMenuCallbacks_s* QBConfigurationMenuCallbacks;

SvGenericObject QBConfigurationMenuDataSourceHandlerCreate(SvWidget menuBar, AppGlobals appGlobals, QBActiveTree menuTree, QBTreePathMap pathMap);

void QBConfigurationMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals);

void QBConfigurationMenuDataSourceHandlerRegisterCallbacks(SvGenericObject dataSourceHandler, SvGenericObject owner, QBConfigurationMenuCallbacks callbacks);

#endif

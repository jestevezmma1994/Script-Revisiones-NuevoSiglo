/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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
#ifndef QBINNOV8ONLOADABLEPANEINTERNAL_H_
#define QBINNOV8ONLOADABLEPANEINTERNAL_H_

#include <main_decl.h>
#include <CUIT/Core/types.h>
#include <ContextMenus/QBInnov8onLoadablePane.h>
#include <ContextMenus/QBLoadablePane.h>
#include <ContextMenus/QBBasicPane.h>
#include <SvFoundation/SvObject.h>
#include <stdbool.h>

struct QBInnov8onLoadablePane_ {
    struct SvObject_ super_;

    QBInnov8onFetchService fetchService; ///< service used for fetching data from MW

    QBLoadablePane loadablePane;
    QBBasicPane basicPane;

    SvWidget connectionErrorPopup;

    bool refreshParams;
    SvObject product;
    SvObject provider;
    size_t productPosition;
    SvObject category;

    QBInnov8onLoadablePaneCallback callback;
    void *callbackData;

    bool loaded;
    int settingsCtx;
    bool refreshObject; ///< flag indicationg if the data obtained from MW should be writen to the source object overwriting old values
    SvApplication app;
};

#endif // QBINNOV8ONLOADABLEPANEINTERNAL_H_

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

#include "QBLatensTestMenu.h"

#include <SvFoundation/SvObject.h>
#include <QBDataModel3/QBActiveTree.h>
#include <main.h>

#define moduleName "QBLatensTestMenu"
#define log_debug(fmt, ...) do { if (1) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ##__VA_ARGS__); } while(0)
#define log_info(fmt, ...) do { if (1) SvLogNotice(moduleName " :: %s " fmt, __func__, ##__VA_ARGS__); } while(0)
#define log_error(fmt, ...) do { if (1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); } while(0)

void QBLatensTestMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    log_debug();
    SvObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("Latens"));

    if (!path)
        return;

    QBTreePathMapInsert(pathMap, path, (SvObject) SVSTRING(""), NULL);

    SVRELEASE(path);
}

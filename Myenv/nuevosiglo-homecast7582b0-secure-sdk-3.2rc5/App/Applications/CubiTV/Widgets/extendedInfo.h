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

#ifndef EXTENDEDINFO_H_
#define EXTENDEDINFO_H_

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>
#include <main_decl.h>
#include <SvDataBucket2/SvDBRawObject.h>

SvWidget QBExtendedInfoNew(SvApplication app, const char *name, AppGlobals appGlobals);
void
QBExtendedInfoSetText(SvWidget w, SvString title, SvString text);
bool
QBExtendedInfoSetTextByVODMetaData(SvWidget w, SvDBRawObject vodMetaData);
void
QBExtendedInfoSetTextByEvent(SvWidget w, SvEPGEvent event, bool force);
void
QBExtendedInfoSetEvents(SvWidget w, SvArray events);
void
QBExtendedInfoSetPosition(SvWidget w, int position, bool force);
void
QBExtendedInfoSetFocus(SvWidget w);
void
QBExtendedInfoRefreshEvents(SvWidget w, SvArray events);
void
QBExtendedInfoReinitialize(SvWidget w);
void
QBExtendedInfoResetPosition(SvWidget w);
void
QBExtendedInfoStartScrollingVertically(SvWidget w, bool downDirection);
void
QBExtendedInfoStopScrollingVertically(SvWidget w);

bool
QBExtendedInfoCanBeScrolledVertically(SvWidget w, bool downDirection);

#endif /* EXTENDEDINFO_H_ */

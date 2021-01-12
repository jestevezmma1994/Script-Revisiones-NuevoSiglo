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

#ifndef TELETEXTWINDOW_H_
#define TELETEXTWINDOW_H_

#include <QBTeletextPageParser.h>
#include <SvFoundation/SvURL.h>
#include <stdbool.h>
#include <main_decl.h>

typedef struct QBTeletextWindow_t *QBTeletextWindow;

void QBTeletextWindowShow(QBTeletextWindow self);
void QBTeletextWindowHide(QBTeletextWindow self);
bool QBTeletextWindowIsVisible(QBTeletextWindow self);

void QBTeletextWindowSetTransparent(QBTeletextWindow self, bool transparent);
bool QBTeletextWindowIsTransparent(QBTeletextWindow self);

QBTeletextWindow QBTeletextWindowNew(AppGlobals appGlobals, QBTeletextReceiver receiver);

void QBTeletextWindowStop(QBTeletextWindow self);
void QBTeletextWindowSubsPage(QBTeletextWindow self, int pageNum);
void QBTeletextWindowSetPage(QBTeletextWindow self, int pageNum);
void QBTeletextWindowSetStartPage(QBTeletextWindow self, struct svdataformat* format, const char *lang);

void QBTeletextWindowSetReceiver(QBTeletextWindow self, QBTeletextReceiver receiver);
void QBTeletextWindowSetHints(QBTeletextWindow self, const char* hintedLang);
#endif /* TELETEXTWINDOW_H_ */

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

#ifndef QB_COUNTDOWN_DIALOG_H_
#define QB_COUNTDOWN_DIALOG_H_

#include <QBApplicationController.h>

#include <stdbool.h>
#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <main_decl.h>

typedef SvString (*QBCountdownDialogSetTimeoutLabel)(int timeout);

typedef struct QBCountdownDialog_ *QBCountdownDialog;

/**
 * Create countdown dialog.
 *
 * @param[in] res                   application handle
 * @param[in] controller            application controller handle
 * @param[in] okMsg                 text to be shown on OK button
 * @param[in] cancelMsg             text to be shown on cancel button
 * @param[in] okFocused             @c true if OK button should be focused, @c if cancel button should be focused
 * @param[in] timeoutLabelGetter    a function that creates a label that changes together with decreasing timeout
 * @return  countdown dialog handle
 **/
QBCountdownDialog
QBCountdownDialogCreate(SvApplication res,
                        QBApplicationController controller,
                        const char *okMsg,
                        const char *cancelMsg,
                        bool okFocused,
                        QBCountdownDialogSetTimeoutLabel timeoutLabelGetter);

void
QBCountdownDialogSetTimeout(QBCountdownDialog self, int timeLeft);

int
QBCountdownDialogGetTimeout(QBCountdownDialog self);

void
QBCountdownDialogSetDelay(QBCountdownDialog self, int delayLeft);

int
QBCountdownDialogGetDelay(QBCountdownDialog self);

void
QBCountdownDialogSetTitle(QBCountdownDialog self, char *title);

SvWidget
QBCountdownDialogGetDialog(QBCountdownDialog self);

SvWidget
QBCountdownDialogAddLabel(QBCountdownDialog self, char *text, SvString tag, unsigned int position);

void
QBCountdownDialogAddTimerLabel(QBCountdownDialog self, SvString tag, unsigned int postion);

#endif

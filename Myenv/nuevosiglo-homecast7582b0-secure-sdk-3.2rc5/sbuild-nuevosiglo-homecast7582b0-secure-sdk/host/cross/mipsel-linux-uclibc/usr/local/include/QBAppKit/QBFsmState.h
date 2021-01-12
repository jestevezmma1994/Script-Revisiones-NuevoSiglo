/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_FSM_STATE_H
#define QB_FSM_STATE_H

#include <QBAppKit/QBFsmStateDefs.h>
#include <QBAppKit/QBFsmTransitionDefs.h>
#include <QBAppKit/QBFsmDefs.h>
#include <QBAppKit/QBFsmCallback.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>

extern QBFsmState
QBFsmStateCreate(SvString name,
                 QBFsmCallback stepInCallback,
                 QBFsmCallback stepOutCallback);

extern void
QBFsmStateAddTransition(QBFsmState self,
                        SvValue transitionSignal,
                        QBFsmTransition transition);

extern QBFsmTransition
QBFsmStateGetTransition(QBFsmState self,
                        SvValue transitionSignal);

extern QBFsmCallback
QBFsmStateGetStepInCallback(QBFsmState self);

extern QBFsmCallback
QBFsmStateGetStepOutCallback(QBFsmState self);

extern void
QBFsmStateSetCallbacks(QBFsmState self,
                       QBFsmCallback stepInCallback,
                       QBFsmCallback stepOutCallback);

extern QBFsmState
QBFsmStateCopyWithPrefix(QBFsmState self,
                         SvString prefix);

extern void
QBFsmStateUpdateTargets(QBFsmState self,
                        SvHashTable mapping);

extern SvString
QBFsmStateGetName(QBFsmState self);

#endif //QB_FSM_STATE_H

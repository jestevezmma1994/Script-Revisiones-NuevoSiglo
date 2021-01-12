
#ifndef QB_AnimationCountdownDialog_H_
#define QB_AnimationCountdownDialog_H_


#include <CUIT/Core/types.h>
#include <SvFoundation/SvString.h>

#include <main.h>
#include <main_decl.h>
#include <stdbool.h>

#include <QBApplicationController.h>


typedef struct AnimationCountdownDialog_ *AnimationCountdownDialog;

SvWidget AnimationCountdownDialogCreate(AppGlobals appGlobals);

void AnimationCountdownDialogExtendTimer(AnimationCountdownDialog self);

#endif

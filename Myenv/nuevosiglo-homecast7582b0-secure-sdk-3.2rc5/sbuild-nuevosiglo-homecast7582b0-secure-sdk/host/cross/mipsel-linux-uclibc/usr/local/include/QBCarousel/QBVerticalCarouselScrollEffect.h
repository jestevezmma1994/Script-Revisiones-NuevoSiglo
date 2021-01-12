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

#ifndef QB_VERTICAL_CAROUSEL_SCROLL_EFFECT_H_
#define QB_VERTICAL_CAROUSEL_SCROLL_EFFECT_H_

/**
 * @file QBVerticalCarouselScrollEffect.h Acceleration and deacceleration scroll effect.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/effect.h>

/**
 * @defgroup QBVerticalCarouselScrollEffect Acceleration and deacceleration scroll effect.
 * @{
 *
 * To start accelerate call QBVerticalCarouselScrollEffectNew. It will accelerate
 * to V.max speed. Offset will come with QBVerticalCarouselScrollEffectStepFn callbacks.
 * To start deacceleration phase call QBVerticalCarouselScrollEffectStop.
 **/

/**
 * Step function called once for each rendered frame
 * during scroll effect activity.
 *
 * @param[in] w      widget scroll effect operates on
 * @param[in] V      scroll effect current speed
 * @param[in] offset offset (in pixels) calculated by the scroll
 *                   effect between previous and current step
 * @return           @c true to redraw screen, @c false otherwise
 **/
typedef bool
(*QBVerticalCarouselScrollEffectStepFn)(SvWidget w, double V,
                        int offset);

/**
 * Scroll effect parameters.
 **/
typedef struct QBVerticalCarouselScrollEffectParams_ {
   /** acceleration factor: A > 0 */
   double A;
   struct {
      /** initial scroll speed in pixels per second */
      double start;
      /** max scroll speed in pixels per second: Vmax >= Vstart */
      double max;
   } V;
   /** scroll effect stop delay */
   int stopOffset;
} QBVerticalCarouselScrollEffectParams;


/**
 * Create scroll effect.
 *
 * @param[in] w      widget for scroll effect to operate on
 * @param[in] params scroll effect parameters
 * @param[in] stepFn step function
 * @return           handle to the created scroll effect,
 *                   @c NULL in case of error
 **/
extern SvEffect
QBVerticalCarouselScrollEffectNew(SvWidget w,
                  QBVerticalCarouselScrollEffectParams params,
                  QBVerticalCarouselScrollEffectStepFn stepFn);

/**
 * Stop scroll effect.
 *
 * This method will signal the effect to start deacceleration phase,
 * which will take approximately the amount of time specified in
 * QBVerticalCarouselScrollEffectParams::Tstop when effect was created.
 *
 * If you want to stop the effect immediately, use svAppCancelEffect().
 *
 * @param[in] scroll scroll effect handle
 **/
extern void
QBVerticalCarouselScrollEffectStop(SvEffect scroll, int stopOffset);

/**
 * @}
 **/

#endif

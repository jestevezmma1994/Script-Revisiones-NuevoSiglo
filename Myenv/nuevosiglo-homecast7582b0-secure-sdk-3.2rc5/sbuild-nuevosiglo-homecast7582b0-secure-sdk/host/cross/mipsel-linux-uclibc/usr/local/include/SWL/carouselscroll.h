/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_GENERAL_SCROLL_EFFECT_H_
#define SV_GENERAL_SCROLL_EFFECT_H_

#include <stdbool.h>
#include <CUIT/Core/types.h>

/**
 * Step function called once for each rendered frame
 * during carousel scroll effect activity.
 *
 * @param[in] w      widget carousel scroll effect operates on
 * @param[in] V      carousel scroll effect current speed
 * @param[in] offset offset (in pixels) calculated by the carousel scroll
 *                   effect between previous and current step
 * @return           @c true to redraw screen, @c false otherwise
 **/
typedef bool
(*SvCarouselScrollEffectStepFn)(SvWidget w, double V, int offset);

/**
 * Scroll effect parameters.
 **/
typedef struct SvCarouselScrollEffectParams_t {
   /** acceleration factor: A > 0 */
   double A;
   /** velocity (speed) of the movement */
   struct {
      /** initial carousel scroll speed in pixels per second */
      double start;
      /** max carousel scroll speed in pixels per second: Vmax >= Vstart */
      double max;
   } V;
   /** carousel scroll effect stop delay */
   int stopOffset;
} SvCarouselScrollEffectParams;


/**
 * Create carousel scroll effect.
 *
 * @param[in] w      widget for carousel scroll effect to operate on
 * @param[in] params carousel scroll effect parameters
 * @param[in] stepFn step function
 * @return           handle to the created carousel scroll effect,
 *                   @c NULL in case of error
 **/
extern SvEffect
SvCarouselScrollEffectNew(SvWidget w,
                          SvCarouselScrollEffectParams params,
                          SvCarouselScrollEffectStepFn stepFn);

/**
 * Stop carousel scroll effect.
 *
 * Stop carousel scroll effect. This method will signal
 * the effect to start deacceleration phase, which will
 * take approximately the amount of time specified in
 * SvCarouselScrollEffectParams::Tstop when effect was created.
 *
 * If you want to stop the effect immediately, use svAppCancelEffect().
 *
 * @param[in] scroll      carousel scroll effect handle
 **/
extern void
SvCarouselScrollEffectStop(SvEffect scroll, int stopOffset);

/**
 * Gets the velocity scroll have had during the last step.
 *
 * Gets the velocity scroll have had during the last step. This may be
 * usefull e.g. if we want to start the new scroll with some initial speed.
 *
 * @param[in] scroll      carousel scroll effect handle
 * @return                velocity during the last step or the initial
 *                        speed passed during the creation if there has not
 *                        been any movement yet
 **/
extern double
SvCarouselScrollGetV(SvEffect scroll);

/**
 * Gets the minimal offset for the gracefull slow-down.
 *
 * Gets the minimal offset for the gracefull slow-down that resembles the
 * speed up. This hint can be ignored, but asking carousel scroll to stop
 * during the offset that is too short would cause the rather "abrupt" stop.
 *
 * @param[in] scroll      carousel scroll effect params handle
 * @return                minimal stopOffset for the gracefull stop
 **/
extern int
SvCarouselScrollGetStopOffset(SvCarouselScrollEffectParams scrollParams);


#endif

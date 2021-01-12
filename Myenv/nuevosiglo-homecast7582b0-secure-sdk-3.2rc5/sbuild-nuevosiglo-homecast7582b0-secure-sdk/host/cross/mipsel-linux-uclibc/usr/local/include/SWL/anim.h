/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2007 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SWL_ANIM_H_
#define SWL_ANIM_H_

#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>

/**
 * @file SMP/SWL/swl/anim.h Animation effect interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

/**
 * @defgroup SvEffectAnim Animation effect
 * @ingroup SWL_effects
 * @{
 *
 * Animation effect performs a simple animation by replacing background bitmap
 * of a widget with consecutive frames of the animation.
 **/


/**
 * Type of the animation effect.
 **/
typedef enum {
    SV_EFFECT_ANIM_FORWARD,         ///< animate from first frame to the last frame
    SV_EFFECT_ANIM_REVERSE,         ///< animate from last frame to the first frame
    SV_EFFECT_ANIM_SWING_FORWARD,   ///< animate from first frame to the last frame and again back to the first frame
    SV_EFFECT_ANIM_SWING_REVERSE    ///< animate from last frame to the first frame and again back to the last frame
} SvEffectAnimType;

/**
 * Mode of animation loop.
 **/
typedef enum {
    /// animate until effect is cancelled
    SV_ANIM_LOOP_INFINITE       = -1,
    /// animation is disabled
    SV_ANIM_LOOP_DISABLE        = 0,
    /// perform animation loop only once
    SV_ANIM_LOOP_SINGLE         = 1
} SvEffectAnimLoopMode;

/**
 * Create animation effect.
 *
 * The effect's animation (list of bitmap frames) will be processed
 * according to the \a type.
 * Processing will loop \a loop times (negative value denotes infinity).
 * One animation loop will last \a duration seconds.
 *
 * @param[in] wgt       target widget
 * @param[in] bmps      an array of animation frames
 * @param[in] count     number of frames in the @a bmps array
 * @param[in] type      the type of animation
 * @param[in] loopMode  animation loop mode
 * @param[in] duration  duration of a single animation loop in seconds
 * @return              newly created effect or @c NULL in case of error
 **/
extern SvEffect
svEffectAnimNew(SvWidget wgt, const SvBitmap *bmps, unsigned int count,
                SvEffectAnimType type, SvEffectAnimLoopMode loopMode,
                double duration);


/**
 * @}
 **/

#endif

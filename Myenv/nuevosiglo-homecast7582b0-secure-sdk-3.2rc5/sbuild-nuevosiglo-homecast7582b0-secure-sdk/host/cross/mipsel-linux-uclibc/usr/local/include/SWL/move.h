/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
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

#ifndef SWL_MOVE_H_
#define SWL_MOVE_H_

/**
 * @file SMP/SWL/swl/move.h Move effect interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>


/**
 * @defgroup SvEffectMove Move effect
 * @ingroup SWL_effects
 * @{
 *
 * Move effect performs animated move of a widget to a final position.
 **/


/**
 * Type describing how the effect step should be calculated.
 **/
typedef enum {
    SV_EFFECT_MOVE_LINEAR,      ///< progress should be a linear function
    SV_EFFECT_MOVE_SLOWIN,      ///< progress should be slower on the begining than on the end
    SV_EFFECT_MOVE_SLOWOUT,     ///< progress should be slower on the end than on the begining
    SV_EFFECT_MOVE_SLOWINOUT,   ///< progress should be faster in the middle than on the edges
    SV_EFFECT_MOVE_LOGARITHM,   ///< progress should be a logarithmic function
} SvEffectMoveType;


/**
 * Type describing direction of move effect, against given x and y
 **/
typedef enum {
    SV_EFFECT_MOVE_PT_FORWARD = 0,      ///< start effect at current position and move widget to P(x,y)
    SV_EFFECT_MOVE_PT_INVERSE,      ///< start effect at P(x,y) and move widget back to current position
    SV_EFFECT_MOVE_VEC_FORWARD,     ///< start effect at current position and move widget by v[x,y]
    SV_EFFECT_MOVE_VEC_INVERSE      ///< given widget is at point P: start effect at P+v[x,y] and move widget back to current position
} SvEffectMoveKinematics;

/**
 * Create new move effect.
 *
 * @param[in] wgt           widget to apply the effect to
 * @param[in] x             effect's X coordinate
 * @param[in] y             effect's Y coordinate
 * @param[in] duration      effect duration in seconds
 * @param[in] type          effect step calculation mode
 * @param[in] direction     effect calculation direction (defaults to 0)
 * @return                  newly created effect or @c NULL in case of error
 **/
extern SvEffect
svEffectMoveNew(SvWidget wgt, int x, int y,
                double duration, SvEffectMoveType type, int direction);


/**
 * Set alignment for move effect.
 *
 * This method sets the alignment values for move for both
 * X and Y directions. Each step of this effect will move
 * a widget by a multiply of the alignment value.
 * Both values are set to @c 1 by default.
 *
 * @param[in] eff           move effect handle
 * @param[in] x_align       alignment in X direction
 * @param[in] y_align       alignment in Y direction
 **/
extern void
svEffectMoveSetAlignment(SvEffect eff, unsigned int x_align, unsigned int y_align);


/**
 * @}
 **/

#endif

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

#ifndef SWL_ROLL_H_
#define SWL_ROLL_H_

/**
 * @file SMP/SWL/swl/roll.h Roll effect interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>

/**
 * @defgroup SvEffectRoll Roll effect
 * @ingroup SWL_effects
 * @{
 *
 * This effect gradually rolls widget to the specified size.
 **/

/**
 * Type of roll effect.
 **/
typedef enum {
    SV_EFFECT_ROLL_LOGARITHM,    ///< progress should be a logarithmic function
    SV_EFFECT_ROLL_LINEAR,       ///< progress should be a linear function
} SvEffectRollType;


/**
 * Create roll effect.
 *
 * @param[in] wgt           widget to roll
 * @param[in] dst_height    final height
 * @param[in] duration      effect duration in seconds
 * @param[in] type          roll effect type
 * @return                  newly created roll effect
 **/
extern SvEffect
svEffectRollNew(SvWidget wgt, int dst_height, double duration, SvEffectRollType type);


/**
 * @}
 **/

#endif

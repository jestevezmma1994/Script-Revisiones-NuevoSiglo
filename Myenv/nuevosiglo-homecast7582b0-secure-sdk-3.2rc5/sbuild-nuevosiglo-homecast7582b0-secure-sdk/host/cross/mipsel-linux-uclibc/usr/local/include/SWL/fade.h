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

#ifndef SWL_FADE_H_
#define SWL_FADE_H_

/**
 * @file SMP/SWL/swl/fade.h Fade effect interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <CAGE/Core/SvColor.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup SvEffectFade Fade effect
 * @ingroup SWL_effects
 * @{
 *
 * Fade effect fades in or fades out a widget by modifying its opacity factor
 * (alpha value).
 **/

/**
 * Type describing how the effect step should be calculated.
 **/
typedef enum {
    SV_EFFECT_FADE_LOGARITHM, ///< progress should be a logarithmic function
    SV_EFFECT_FADE_LINEAR     ///< progress should be a linear function
} SvEffectFadeType;


/**
 * Create fade effect.
 *
 * @param[in] wgt           widget to apply the effect to
 * @param[in] final_alpha   final alpha value of the widget
 * @param[in] tree          if @c true, effect will also be applied to it's children
 * @param[in] duration      effect duration in seconds
 * @param[in] type          effect step calculation mode
 * @returns                 newly created effect or @c NULL in case of error
 **/
extern SvEffect
svEffectFadeNew(SvWidget wgt, SvPixel final_alpha, bool tree,
                double duration, SvEffectFadeType type);


/**
 * @}
 **/

#endif

/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
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

#ifndef SWL_TRANSFORM_H_
#define SWL_TRANSFORM_H_

/**
 * @file SMP/SWL/swl/transform.h Transform effect interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>

/**
 * @defgroup SvEffectTransform Transform effect
 * @ingroup SWL_effects
 * @{
 *
 * Transform effect gradually changes size and position of the widget until it reaches
 * final size and position.
 **/

/**
 * Type describing how the effect step should be calculated.
 **/
typedef enum {
    SV_EFFECT_TRANSFORM_LINEAR,     ///< progress should be a linear function
    SV_EFFECT_TRANSFORM_SLOWIN,     ///< progress should be slower on the begining than on the end
    SV_EFFECT_TRANSFORM_SLOWOUT,    ///< progress should be slower on the end than on the begining
    SV_EFFECT_TRANSFORM_SLOWINOUT,  ///< progress should be faster in the middle than on the edges
    SV_EFFECT_TRANSFORM_LOGARITHM,  ///< progress should be a logarithmic function
} SvEffectTransformType;


/**
 * Create transform effect.
 *
 * @param[in] wgt           widget to transform
 * @param[in] dst_x         final widget X coordinate
 * @param[in] dst_y         final widget Y coordinate
 * @param[in] dst_w         final widget width
 * @param[in] dst_h         final widget height
 * @param[in] duration      effect duration in seconds
 * @param[in] type          transform type
 * @return                  newly created transform effect, @c NULL in case of error
 */
extern SvEffect
svEffectTransformNew(SvWidget wgt, int dst_x, int dst_y, int dst_w, int dst_h,
                     double duration, SvEffectTransformType type);


/**
 * @}
 **/

#endif


/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SWL_ROTATE_H_
#define SWL_ROTATE_H_

/**
 * @file SMP/SWL/swl/rotate.h Rotate effect interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <CUIT/Core/types.h>
#include <SvGfxHAL/Sv3DPoint.h>

/**
 * @defgroup SvEffectRotate Rotate effect
 * @ingroup SWL_effects
 * @{
 *
 * Rotate effect rotates widget from specified angle to another one around specified axis
 **/

/**
 * Type describing how the effect step should be calculated.
 **/
typedef enum {
    SV_EFFECT_ROTATE_LOGARITHM, ///< progress should be a logarithmic function
    SV_EFFECT_ROTATE_LINEAR     ///< progress should be a linear function
} SvEffectRotateType;


/**
 * Create rotate effect.
 *
 * @param[in] wgt           widget to apply the effect to
 * @param[in] startAngle    start angle of widget relative to the specified axis
 * @param[in] endAngle      end angle of widget relative to the specified axis
 * @param[in] axis          axis specified by 2 points in 3d space
 * @param[in] duration      effect duration in seconds
 * @param[in] type          effect step calculation mode
 * @returns                 newly created effect or @c NULL in case of error
 **/
extern SvEffect
svEffectRotateNew(SvWidget wgt, float startAngle, float endAngle, Sv3DPoint axis[2], double duration, SvEffectRotateType type);


/**
 * @}
 **/

#endif

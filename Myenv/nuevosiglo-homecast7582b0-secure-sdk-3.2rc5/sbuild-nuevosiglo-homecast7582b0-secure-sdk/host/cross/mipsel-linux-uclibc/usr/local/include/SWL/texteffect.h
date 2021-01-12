/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SWL_TEXT_EFFECT_H_
#define SWL_TEXT_EFFECT_H_

/**
 * @file SMP/SWL/swl/texteffect.h Text effect interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <CAGE/Core/SvColor.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup SvEffectText Text effect
 * @ingroup SWL_effects
 * @{
 *
 * Text effect performs a smooth transition of widget bitmap's color
 * to a specified final color.
 **/

/**
 * Create text effect.
 *
 * @param[in] wgt           widget to apply the effect to
 * @param[in] final_color   final color of the widget bitmap
 * @param[in] duration      effect duration in seconds
 * @return                  newly created effect or @c NULL in case of error
 */
extern SvEffect
svEffectTextNew(SvWidget wgt, SvColor final_color, double duration);


/**
 * @}
 **/

#endif

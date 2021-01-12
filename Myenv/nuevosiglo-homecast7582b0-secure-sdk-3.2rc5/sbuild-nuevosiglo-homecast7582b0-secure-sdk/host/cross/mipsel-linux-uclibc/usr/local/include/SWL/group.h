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

#ifndef SWL_GROUP_H_
#define SWL_GROUP_H_

/**
 * @file SMP/SWL/swl/group.h Group effect interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <stdbool.h>
#include <CUIT/Core/types.h>


/**
 * @defgroup SvEffectGroup Group effect
 * @ingroup SWL_effects
 * @{
 *
 * This is a dummy effect that doesn't do anything by itself;
 * its purpose is to group other effects so that they are computed
 * in parallel.
 **/

/**
 * Create new group effect.
 *
 * @param[in] app           CUIT application handle
 * @param[in] notify        if @c true, effect will send notifications on every step
 *                          (it affects performance)
 * @return                  newly created effect or @c NULL in case of error
 **/
extern SvEffect
svEffectGroupNew(SvApplication app, bool notify);

/**
 * Add an effect to the group.
 *
 * @param[in] grp            group effect handle
 * @param[in] eff            effect to be added to the group
 * @return                   @c 1 on success, @c 0 on error
 **/
extern int
svEffectGroupAddEffect(SvEffect grp, SvEffect eff);

/**
 * Remove an effect from the group.
 *
 * @param[in] grp            group effect handle
 * @param[in] eff            effect to be removed from the group
 * @return                   @c 1 on success, @c 0 on error
 **/
extern int
svEffectGroupRemoveEffect(SvEffect grp, SvEffect eff);


/**
 * @}
 **/

#endif

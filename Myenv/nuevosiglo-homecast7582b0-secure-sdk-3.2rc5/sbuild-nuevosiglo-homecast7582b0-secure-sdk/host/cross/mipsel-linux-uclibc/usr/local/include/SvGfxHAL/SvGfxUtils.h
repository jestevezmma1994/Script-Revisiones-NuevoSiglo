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

#ifndef SV_GFX_UTILS_H_
#define SV_GFX_UTILS_H_

/**
 * @file SvGfxUtils.h GFX HAL utility functions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvBitmap.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvGfxUtils GFX HAL utility functions
 * @ingroup SvGfxHAL
 * @{
 *
 * Utility functions simplifying common tasks.
 **/

/**
 * Copy a region of a bitmap into GFX surface, allocating
 * the GFX surface if needed.
 *
 * @param[in] src       source host bitmap handle
 * @param[in] srcRect   region of the @a src bitmap, that is to be copied to
 *                      GFX surface, pass @c NULL to copy entire bitmap
 **/
extern void
SvGfxBlitBitmap(SvBitmap src, const Sv2DRect *srcRect);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

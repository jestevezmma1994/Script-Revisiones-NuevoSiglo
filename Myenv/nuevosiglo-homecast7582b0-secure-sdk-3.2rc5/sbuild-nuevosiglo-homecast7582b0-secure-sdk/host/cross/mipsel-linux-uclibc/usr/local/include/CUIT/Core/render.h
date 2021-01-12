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

#ifndef CUIT_RENDER_H_
#define CUIT_RENDER_H_

/**
 * @file SMP/CUIT/render.h CUIT scene rendering
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvApplicationRendering Rendering scene
 * @ingroup SvApplication
 * @{
 *
 * Scene rendering module provides an interface to render part of
 * application's scene to a GFX surface using hardware acceleration.
 **/

/**
 * Render a tree of widgets to GFX surface.
 *
 * @param[in] app CUIT application handle
 * @param[in] dest target surface
 * @param[in] root root of the widgets tree
 * @param[in] destRect destination rectangle, pass @c NULL for entire surface
 * @param[in] clear @c true to clear @a dest before rendering
 * @return length of queue rendered by GFX engine, @c -1 in case of error
 **/
extern int
svAppRenderOffscreen(SvApplication app,
                     SvHWSurface dest, SvWidget root,
                     const Sv2DRect *destRect,
                     bool clear);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

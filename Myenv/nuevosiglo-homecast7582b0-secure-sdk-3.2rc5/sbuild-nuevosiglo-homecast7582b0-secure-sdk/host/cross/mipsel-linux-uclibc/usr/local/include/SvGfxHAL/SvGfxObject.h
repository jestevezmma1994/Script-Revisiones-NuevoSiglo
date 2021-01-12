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

#ifndef SV_GFX_OBJECT_H_
#define SV_GFX_OBJECT_H_

/**
 * @file SvGfxObject.h Declaration of the GFX object (operation) data type
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvColor.h>
#include <SvGfxHAL/SvHWSurface.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup SvGfxQueue
 * @{
 **/

/**
 * Single GFX operation.
 **/
typedef struct SvGfxObject_s {
    /**
     * unique object identifier, @c 0 iff unknown; used for quick checking
     * if a specific object is in the queue, even if it's changed
     **/
    uint32_t  object_id;

    /// @c true to blend @a src to destination, @c false to copy
    bool  blend;

    /// source GFX surface handle
    SvHWSurface  src;
    /// counter increased each time SvGfxObject_s::src surface is modified
    uint32_t  src_generation_counter;

    /// source rectangle
    Sv2DRect src_rect;

    /// destination rectangle
    Sv2DRect dest_rect;

    /// color of the source GFX surface
    SvColor  color;

    /**
     * global brightness adjustment:
     * @c -127 to @c -1 to darken, @c 0 is a no-op, @c 1 to @c 127 to lighten
     **/
    int32_t ldim;
} SvGfxObject;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

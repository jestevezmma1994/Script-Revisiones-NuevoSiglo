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

#ifndef SV_HW_SURFACE_INTERNAL_H_
#define SV_HW_SURFACE_INTERNAL_H_

/**
 * @file SvHWSurface.h GFX surface class private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvSurface.h>
#include <CAGE/Core/SvBitmap.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup SvHWSurface
 * @{
 **/

/**
 * SvHWSurface class internals.
 * @private
 **/
struct SvHWSurface_ {
    /// super class
    struct SvSurface_ super_;
};

/**
 * SvHWSurface virtual methods table.
 **/
typedef const struct SvHWSurfaceVTable_ {
    /// virtual methods of the base class
    struct SvObjectVTable_ super_;

    /**
     * Allocate surface in GFX engine memory.
     *
     * @param[in] self      GFX surface handle
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*alloc)(SvHWSurface self);

    /**
     * Copy a region of a bitmap into GFX surface.
     *
     * @param[in] self      destination GFX surface handle
     * @param[in] destX     horizontal position of destination's upper left corner
     * @param[in] destY     vertical position of destination's upper left corner
     * @param[in] src       source bitmap handle
     * @param[in] srcRect   region of the @a src bitmap, that is to be copied to
     *                      @a dest surface
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*upload)(SvHWSurface self,
                  unsigned int destX,
                  unsigned int destY,
                  const SvBitmap src,
                  const Sv2DRect *srcRect);

    /**
     * Read a region of a GFX surface back to the software surface.
     *
     * @param[in] self      source GFX surface
     * @param[in] srcRect   region of the @a src surface, that is to be copied
     *                      back to @a dest bitmap
     * @param[in] dest      destination bitmap
     * @param[in] destX     horizontal position of destination's upper left corner
     * @param[in] destY     vertical position of destination's upper left corner
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*readBack)(SvHWSurface self,
                    const Sv2DRect *srcRect,
                    SvBitmap dest,
                    unsigned int destX,
                    unsigned int destY);
} *SvHWSurfaceVTable;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CAGE_SURFACE_H_
#define CAGE_SURFACE_H_

/**
 * @file SvSurface.h Surface class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvSurfaceIterator.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvSurface Base surface class
 * @ingroup CAGE
 * @{
 *
 * SvSurface is a base abstract class for other surface classes,
 * most notably the @ref SvBitmap and @ref SvHWSurface.
 **/

/**
 * Base surface class.
 * @class SvSurface
 * @extends SvObject
 **/
typedef struct SvSurface_ {
    /// super class
    struct SvObject_ super_;

    /// surface width in pixels
    unsigned int width;
    /// surface height in pixels
    unsigned int height;
    /// color space
    SvColorSpace cs;

    /// this counter changes every time the surface's contents changes
    uint32_t generationCounter;

    /// number of bytes in each allocated row of pixels
    size_t pitch;
    /// pixel data
    SvPixel *data;
} *SvSurface;


/**
 * Return the runtime type identification object
 * of the SvSurface class.
 *
 * @return surface class
 **/
extern SvType
SvSurface_getType(void);

/**
 * Initialize surface object with given attributes.
 *
 * @memberof SvSurface
 *
 * @param[in] self      software surface object handle
 * @param[in] width     surface width
 * @param[in] height    surface height
 * @param[in] csp       color space
 * @return              @a self or @c NULL in case of error
 **/
extern SvSurface
SvSurfaceInit(SvSurface self,
              unsigned int width, unsigned int height,
              SvColorSpace csp);

/**
 * Create surface iterator at given coordinates.
 *
 * @memberof SvSurface
 *
 * @param[in] self      surface handle
 * @param[in] x         initial X coordinate (column number)
 * @param[in] y         initial Y coordinate (row number)
 * @return              iterator
 **/
static inline SvSurfaceIterator
SvSurfaceGetIterator(SvSurface self,
                     unsigned int x,
                     unsigned int y)
{
    SvSurfaceIterator iterator = {
        .data  = self->data + y * self->pitch + x * BPP(self->cs),
        .pitch = self->pitch,
        .cs    = self->cs
    };
    return iterator;
}

/**
 * @}
 **/


/**
 * GFX memory surface class.
 * @class SvHWSurface
 * @extends SvSurface
 * @ingroup SvHWSurface
 **/
typedef struct SvHWSurface_ *SvHWSurface;


#ifdef __cplusplus
}
#endif

#endif

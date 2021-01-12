/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CAGE_SURFACE_ITERATOR_H_
#define CAGE_SURFACE_ITERATOR_H_

/**
 * @file SvSurfaceIterator.h Surface iterator API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <CAGE/Core/SvColor.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvSurfaceIterator Surface iterator
 * @ingroup CAGE
 * @{
 *
 * SvSurfaceIterator maintains information about a position in the array
 * of pixels, that form surface data, and information necessary to update
 * this position.
 **/

/**
 * Surface iterator.
 **/
typedef struct {
    /// position in surface data
    SvPixel *data;
    /// pitch (stride) of the surface data
    unsigned int pitch;
    /// format of surface pixels
    SvColorSpace cs;
} SvSurfaceIterator;


/**
 * Copy an area of source surface pointed to by @a src iterator
 * to an area of destination surface pointed to by @a dest iterator.
 *
 * This method applies a function returned by SvBitmapGetPixelCopyFn()
 * to surface areas pointed to by two iterators.
 *
 * @param[in] dest      destination surface iterator
 * @param[in] src       source surface iterator
 * @param[in] srcColor  source pixel color used when source surface
 *                      in alpha-only (@ref SvColorSpace::SV_CS_MONO)
 * @param[in] width     width of the copied area, in pixels
 * @param[in] height    height of the copied area, in pixels
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
SvSurfaceIteratorCopyPixels(const SvSurfaceIterator *dest,
                            const SvSurfaceIterator *src,
                            SvColor srcColor,
                            unsigned int width,
                            unsigned int height);

/**
 * Multiply color components of surface pixels by their alpha channel.
 *
 * This method applies a function returned by SvBitmapGetColorPremultiplyFn()
 * to surface area pointed to by an iterator.
 *
 * @param[in] iter      surface iterator
 * @param[in] width     width of the processed area, in pixels
 * @param[in] height    height of the processed area, in pixels
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
SvSurfaceIteratorPremultiplyColor(const SvSurfaceIterator *iter,
                                  unsigned int width,
                                  unsigned int height);

/**
 * Divide color components of surface pixels by their alpha channel.
 *
 * This method applies a function returned by SvBitmapGetColorUnassociateFn()
 * to surface area pointed to by an iterator.
 *
 * @param[in] iter      surface iterator
 * @param[in] width     width of the processed area, in pixels
 * @param[in] height    height of the processed area, in pixels
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
SvSurfaceIteratorUnassociateColor(const SvSurfaceIterator *iter,
                                  unsigned int width,
                                  unsigned int height);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

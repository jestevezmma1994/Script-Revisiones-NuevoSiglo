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

#ifndef CAGE_2D_RECT_H_
#define CAGE_2D_RECT_H_

/**
 * @file Sv2DRect.h 2D rectangle class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup Sv2DRect 2D rectangle class
 * @ingroup CAGE
 * @{
 *
 * Sv2DRect class defines a rectangular area in 2D coordinates system.
 **/


/**
 * 2D rectangle class.
 *
 * @note This is not an SvObject.
 **/
typedef struct Sv2DRect_ {
    /// rectangle width in pixels
    uint32_t width;
    /// rectangle height in pixels
    uint32_t height;
    /// X coordinate of rectangle's top left corner, in pixels
    int32_t x;
    /// Y coordinate of rectangle's top left corner, in pixels
    int32_t y;
} Sv2DRect;


/**
 * 2D rectangle constructor, lets you initialize a variable
 * of Sv2DRect type in one line of code.
 *
 * @param[in] x         X coordinate of rectangle's top left corner, in pixels
 * @param[in] y         Y coordinate of rectangle's top left corner, in pixels
 * @param[in] width     rectangle width in pixels
 * @param[in] height    rectangle height in pixels
 * @return              rectangle
 **/
static inline Sv2DRect
Sv2DRectCreate(int x, int y, unsigned int width, unsigned int height)
{
    Sv2DRect rect;

    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;

    return rect;
}

/**
 * Check if two rectangles are equal (have the same
 * size and position).
 *
 * @param[in] rectA     first rectangle
 * @param[in] rectB     second rectangle
 * @return              @c true if @a rectA and @a rectB are equal
 **/
static inline bool
Sv2DRectIsEqual(const Sv2DRect *const rectA,
                const Sv2DRect *const rectB)
{
    return    rectA->x == rectB->x
           && rectA->y == rectB->y
           && rectA->width == rectB->width
           && rectA->height == rectB->height;
}

/**
 * Check if two rectangles intersect (have common part).
 *
 * @param[in] rectA     first rectangle
 * @param[in] rectB     second rectangle
 * @return              @c true if @a rectA and @a rectB intersect
 **/
extern bool
Sv2DRectIsIntersecting(const Sv2DRect *const rectA,
                       const Sv2DRect *const rectB);

/**
 * Compute intersection of two rectangles.
 *
 * @param[in,out] dest  first rectangle, will contain intersection
 *                      value on return
 * @param[in] rectB     second rectangle
 **/
extern void
Sv2DRectIntersect(Sv2DRect *const dest,
                  const Sv2DRect *const rectB);

/**
 * Check if rectangle entirely contains another rectangle.
 *
 * @param[in] rectA     first rectangle
 * @param[in] rectB     second rectangle
 * @return              @c true if @a rectA entirely contains @a rectB
 **/
extern bool
Sv2DRectIsContaining(const Sv2DRect *const rectA,
                     const Sv2DRect *const rectB);

/**
 * Compute sum (union) of two rectangles.
 *
 * @param[in,out] dest  first rectangle, will contain sum on return
 * @param[in] rectB     second rectangle
 **/
extern void
Sv2DRectAdd(Sv2DRect *const dest,
            const Sv2DRect *const rectB);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

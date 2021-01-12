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

#ifndef SV_GFX_3D_POINT_H_
#define SV_GFX_3D_POINT_H_

/**
 * @file Sv3DPoint.h 3D point class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup Sv3DPoint 3D point class
 * @ingroup SvGfxEngine
 * @{
 *
 * Sv3DPoint class defines coordinates of a point in a 3D space.
 **/


/**
 * 3D point class.
 *
 * @note This is not an SvObject!
 **/
typedef struct {
    /// X coordinate
    int x;
    /// Y coordinate
    int y;
    /// Z coordinate
    float z;
} Sv3DPoint;

/**
 * 3D point constructor, lets you initialize a variable
 * of Sv3DPoint type in one line of code.
 *
 * @param[in] x         X coordinate of rectangle's top left corner, in pixels
 * @param[in] y         Y coordinate of rectangle's top left corner, in pixels
 * @param[in] z         Z coordinate in unit relative to screen width (1.0f is screen width)
 * @return              point
 **/
static inline Sv3DPoint
Sv3DPointCreate(int x, int y, float z)
{
    Sv3DPoint point;

    point.x = x;
    point.y = y;
    point.z = z;

    return point;
}

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

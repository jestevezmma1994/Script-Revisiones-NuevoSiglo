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

#ifndef SV_GFX_3D_OPERATION_H_
#define SV_GFX_3D_OPERATION_H_

/**
 * @file Sv3DOperation.h 3D operation related definitions
 **/

#include <SvGfxHAL/Sv3DPoint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Sv3DOperation related definitions
 * @ingroup SvGfxEngine
 * @{
 *
 **/

/**
 * Types of 3D operations.
 **/
typedef enum {
    Sv3DOperationType_none,                  ///< no 3D operation is defined
    Sv3DOperationType_pop,                   ///< 3D operation pop from operations stack
    Sv3DOperationType_rotate,                ///< 3D rotate operation
    Sv3DOperationType_move,                  ///< 3D move operation
    Sv3DOperationType_perspective_default,   ///< 3D operation restore default perspective
    Sv3DOperationType_perspecitve_lookAt,    ///< 3D operation changing perspective in such way that the camera looks straight at the given point
} Sv3DOperationType;

/**
 * Rotate operation definition.
 **/
typedef struct {
    float angle;        ///< rotate angle relative to the axis
    Sv3DPoint axis[2];  ///< Axis defined by 2 points in space
} Sv3DOperationRotate;

/**
 * Move operation definition.
 **/
typedef struct {
    int x;              ///< move in x axis direction in px
    int y;              ///< move in y axis direction in px
    float z;            ///< move in z axis direction relative to screen width
} Sv3DOperationMove;

/**
 * Camera look at operation definition.
 **/
typedef struct {
    float x;              ///< move in x axis direction in px
    float y;              ///< move in y axis direction in px
} Sv3DOperationPerspectiveLookAt;

/**
 * 3D operation definition.
 **/
typedef struct Sv3DOperation_s {
    Sv3DOperationType type;              ///< operation type
    union {                              ///< union with possible types of operations
        Sv3DOperationRotate rotate;
        Sv3DOperationMove move;
        Sv3DOperationPerspectiveLookAt lookAt;
    };
} Sv3DOperation;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

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

#ifndef SV_GFX_COLOR_OPERATION_H_
#define SV_GFX_COLOR_OPERATION_H_

/**
 * @file SvColorOperation.h Color operation related definitions
 **/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvColorOperation Color operations related definitions
 * @ingroup SvGfxEngine
 * @{
 *
 **/

/**
 * Multiplier for fixed point operations
 **/
#define SV_COLOR_OPERATION_SHIFT (12)

/**
 * Types of color operations.
 **/
typedef enum {
    SvColorOperationType_none,       ///< no color operation is defined
    SvColorOperationType_pushMatrix, ///< push color operations matrix to operations stack
    SvColorOperationType_popMatrix,  ///< pop color operation matrix from operations stack
    SvColorOperationType_cnt,        ///< color operation types count
} SvColorOperationType;

/**
 * Color operation definition.
 *
 * @note Currently it's assumed that whole screen should be redrawn if color
 * operation was found in SvGfxQueue. See CUIT/Core/render.c : detectChanges()
 * and SvGfxQueueClip().
 **/
typedef struct SvColorOperation_s {
    SvColorOperationType type; ///< operation type
    int16_t colorMatrix[3][4]; ///< 3 rows x 4 columns, Q4.12 fixed point numbers
} SvColorOperation;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // SV_GFX_COLOR_OPERATION_H_

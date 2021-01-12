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

#ifndef CAGE_ERROR_DOMAIN_H_
#define CAGE_ERROR_DOMAIN_H_

/**
 * @file CAGEErrorDomain.h CAGE error domain
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup CAGEErrorDomain CAGE error domain
 * @ingroup CAGE
 * @{
 **/

/**
 * CAGE error domain.
 **/
#define CAGEErrorDomain "com.cubiware.CAGE"

/**
 * CAGE error domain error codes.
 **/
typedef enum {
    /** @cond */
    CAGEError_OK = 0,
    /** @endcond */
    /// some unknown error occured while loading bitmap from file
    CAGEError_unableToLoadBitmap = 1,
    /// invalid format of bitmap file
    CAGEError_invalidBitmapFormat,
    /// unsupported format of bitmap file
    CAGEError_unsupportedBitmapFormat,
    /// I/O error while writing image file
    CAGEError_writeIOError,
    /// invalid input to text layout engine
    CAGEError_invalidTextInput,
    /// box to small to fit text paragraph
    CAGEError_unsufficientDimensions,
    /// invalid source color for operation on MONO bitmap
    CAGEError_invalidSourceColor,
    /// unsupported color space for some operation on bitmap
    CAGEError_unsupportedColorSpace,
    /** @cond */
    CAGEError_last = CAGEError_unsupportedColorSpace
    /** @endcond */
} CAGEError;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

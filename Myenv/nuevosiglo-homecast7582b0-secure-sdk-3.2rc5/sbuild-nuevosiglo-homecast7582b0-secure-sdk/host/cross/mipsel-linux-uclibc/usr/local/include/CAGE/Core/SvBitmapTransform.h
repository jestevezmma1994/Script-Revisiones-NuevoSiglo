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
 ** includes, but is not limited to
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef CAGE_BITMAP_Transform_H_
#define CAGE_BITMAP_Transform_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/SvBitmap.h>

/**
 * SvTransform class
 * @class SvBitmapTransform
 */
typedef struct SvBitmapTransform_ *SvBitmapTransform;

/**
 * Return the runtime type identification object
 * of the SvBitmapTransform class.
 *
 * @return SvBitmapTransform class
 **/
extern SvType
SvBitmapTransform_getType(void);

/**
 * Create new SvBitmapTransform
 *
 * @memberof SvBitmapTransform
 *
 * @param[in] bmp        target bitmap
 * @param[in] srcWidth   source bitmap width
 * @param[in] srcHeight  source bitmap height
 * @param[in] scsp       source color space
 * @param[in] srcPitch   source pitch
 * @return               newly created SvBitmapTransform or @c NULL in case of error
 **/
extern SvBitmapTransform
SvBitmapTransformCreate(SvBitmap bmp,
        unsigned int srcWidth, unsigned int srcHeight, SvColorSpace scsp, size_t srcPitch);

/**
 * Copies data from row of pixels (obtained from SvBitmapTransformGetRowStorage) to proper row in bitmap
 *
 * @memberof SvBitmapTransform
 *
 * @param[in]     self           bitmap transform handle
 * @param[in]     rowNumber      number of row where put data
 * @param[out]    errorOut       error info
 **/
extern void
SvBitmapTransformPushRow(SvBitmapTransform self, unsigned int rowNumber, SvErrorInfo *errorOut);

/**
 * Returns row of pixels where data should be put before transformation
 *
 * @memberof SvBitmapTransform
 *
 * @param[in]     self         bitmap transform handle
 * @return                     row of picture were you can write data for transform
 **/
extern SvPixel*
SvBitmapTransformGetRowStorage(SvBitmapTransform self);

#ifdef __cplusplus
}
#endif

#endif

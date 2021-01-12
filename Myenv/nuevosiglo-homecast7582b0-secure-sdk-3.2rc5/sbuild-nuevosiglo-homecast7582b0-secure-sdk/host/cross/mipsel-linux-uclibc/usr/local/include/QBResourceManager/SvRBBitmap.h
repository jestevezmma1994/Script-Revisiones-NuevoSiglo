/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
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

#ifndef SV_RB_BITMAP_H_
#define SV_RB_BITMAP_H_

/**
 * @file SvRBBitmap.h
 * @brief Resource manager bitmap class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <QBResourceManager/SvRBObject.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvRBBitmap Resource manager bitmap class
 * @ingroup QBResourceManagerLibrary
 * @{
 *
 * A class representing bitmap managed as the resource manager object.
 *
 * @image html SvRBBitmap.png
 **/

/**
 * Resource manager bitmap class.
 * @class SvRBBitmap SvRBBitmap.h <QBResourceManager/SvRBBitmap.h>
 * @extends SvRBObject
 **/
typedef struct SvRBBitmap_ *SvRBBitmap;


/**
 * Get runtime type identification object representing type
 * of resource manager bitmap class.
 *
 * @relates SvRBBitmap
 *
 * @return resource manager bitmap class
 **/
extern SvType SvRBBitmap_getType(void);

/**
 * Create new resource manager bitmap object.
 *
 * This method creates new resource manager object representing CAGE bitmap.
 * Created object can then be registered in the resource manager service, which
 * keeps track of its registered resources (represented as resource manager
 * objects) and their usage, freeing them when appropriate.
 *
 * @memberof SvRBBitmap
 *
 * @param[in] URI       URI of the source file, often a local path
 * @param[in] type      bitmap type
 * @return              created object or @c NULL in case of error
 **/
extern SvRBBitmap SvRBBitmapCreateWithURI(SvURI URI,
                                          SvBitmapType type);

/**
 * Create new resource manager bitmap object.
 *
 * This is a convenience wrapper for SvRBBitmapCreateWithURI().
 *
 * @memberof SvRBBitmap
 *
 * @param[in] URI       URI of the source file, often a local path
 * @param[in] type      bitmap type
 * @return              created object or @c NULL in case of error
 **/
extern SvRBBitmap SvRBBitmapCreateWithURICString(const char *URI,
                                                 SvBitmapType type);

/**
 * Create new resource manager bitmap object.
 *
 * This method is an old deprecated wrapper for
 * SvRBBitmapCreateWithURICString() and SvRBBitmapSetDestSize().
 * It should not be used in new code.
 *
 * @memberof SvRBBitmap
 *
 * @param[in] rid       deprecated parameter, pass @c SV_RID_INVALID
 * @param[in] URI       URI of the source file, often a local path
 * @param[in] type      bitmap type
 * @param[in] width     desired bitmap width in pixels
 * @param[in] height    desired bitmap height in pixels
 * @return              created object or @c NULL in case of error
 **/
extern SvRBBitmap SvRBBitmapCreate(SvRID rid, const char *URI,
                                   SvBitmapType type,
                                   int width, int height);

/**
 * Create new resource manager bitmap object from existing bitmap.
 *
 * @memberof SvRBBitmap
 *
 * @param[in] bitmap    bitmap handle
 * @return              created object or @c NULL in case of error
 **/
extern SvRBBitmap SvRBBitmapCreateWithBitmap(SvBitmap bitmap);

/**
 * Set requested size of the resource manager bitmap.
 *
 * This method sets the requested size of the bitmap. It must be called
 * before bitmap is loaded.
 *
 * You can specify both @a width and @a height of a bitmap. If only one of
 * those values is valid (greater than @c 0), the other value will be
 * choosen to keep the aspect ratio of the original. When both @a width and
 * @a height are less or equal @a 0, bitmap will be kept at its original size.
 *
 * @note Using this method will cause the bitmap to be scaled in software.
 *       It should be used with caution only when it is really necessary.
 *
 * @memberof SvRBBitmap
 *
 * @param[in] self      resource manager bitmap object handle
 * @param[in] width     desired bitmap width in pixels
 * @param[in] height    desired bitmap height in pixels
 **/
extern void SvRBBitmapSetDestSize(SvRBBitmap self,
                                  int width, int height);

/**
 * Check if resource manager bitmap object matches given parameters.
 *
 * @memberof SvRBBitmap
 *
 * @param[in] self      resource manager bitmap object handle
 * @param[in] type      bitmap type
 * @param[in] csp       bitmap color space
 * @param[in] width     desired bitmap width in pixels,
 *                      @c 0 if not important
 * @param[in] height    desired bitmap height in pixels,
 *                      @c 0 if not important
 * @return              @c true if bitmap matches
 *                      @a type, @a csp, @a width and @a height
 **/
extern bool SvRBBitmapIsCompatible(SvRBBitmap self,
                                   SvBitmapType type,
                                   SvColorSpace csp,
                                   int width, int height);

/**
 * Set desired color space for bitmap.
 *
 * This method can be used before actual bitmap is loaded
 * to force resource manager to convert it from its native
 * color space (the one in which bitmap is represented
 * in the image file) to other color space.
 *
 * @memberof SvRBBitmap
 *
 * @param[in] self      resource manager bitmap object handle
 * @param[in] csp       desired color space
 **/
extern void SvRBBitmapSetColorSpace(SvRBBitmap self,
                                    SvColorSpace csp);

/**
 * Request asynchronous loading mode for a bitmap.
 *
 * @memberof SvRBBitmap
 *
 * @param[in] self      resource manager bitmap object handle
 * @param[in] async     @c true to enable asynchronous loading
 **/
extern void SvRBBitmapSetAsync(SvRBBitmap self,
                               bool async);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

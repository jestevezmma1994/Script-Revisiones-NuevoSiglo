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

#ifndef TIVO_IMAGE_DB_H_
#define TIVO_IMAGE_DB_H_

/**
 * @file TiVoImageDB.h
 * @brief Images database for TiVo
 *
 * Module allowing for handling metadata of images of different sizes/aspect ratios
 * provided by TiVo images database.
 **/

/**
 * @defgroup TiVoImageDB
 * @ingroup QBMWClient
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvURL.h>

/**
 * TiVo images database image format.
 **/
typedef enum {
    TiVoImageDBImageFormat_unknown = -1,
    TiVoImageDBImageFormat_jpg = 0,
    TiVoImageDBImageFormat_jpeg = 1,
    TiVoImageDBImageFormat_default = TiVoImageDBImageFormat_jpg
} TiVoImageDBImageFormat;

/**
 * TiVo image parameters class.
 *
 * This class is responsible for providing parameters of image
 * of certain size and type.
 *
 * @class TiVoImageDBImageParams
 * @extends SvObject
 **/
typedef struct TiVoImageDBImageParams_ *TiVoImageDBImageParams;


/**
 * TiVo images database class.
 *
 * This class is responsible for handling TiVo images metadata.
 *
 * @class TiVoImageDB
 * @extends SvObject
 **/
typedef struct TiVoImageDB_ *TiVoImageDB;


/**
* Get runtime type identification object representing TiVoImageDB class.
*
* @return TiVo images database class
**/
SvType
TiVoImageDB_getType(void);


/**
 * A function initializing TiVo images database.
 *
 * @param[in]  self             pointer to TiVo images database
 * @param[in]  baseUrl          images base URL
 * @param[in]  imagesParamsSet  array of handled images parameters @ref TiVoImageDBImageParams
 * @param[out] errorOut         error info
 * @return                      @c 0 on success, @c -1 in case of error
 **/
int
TiVoImageDBInit(TiVoImageDB self, SvURL baseUrl, SvArray imagesParamsSet, SvErrorInfo *errorOut);

/**
 * A function extending object data with proper images metadata.
 *
 * @param[in]  self             pointer to TiVo images database
 * @param[in]  object           object to be extended
 * @param[out] errorOut         error info
 * @return                      @c 0 on success, @c -1 in case of error
 **/
int
TiVoImageDBAdaptObject(TiVoImageDB self, SvDBRawObject object, SvErrorInfo *errorOut);

/**
 * A function creating TiVo image parameters.
 *
 * @param[in]  width            image width
 * @param[in]  height           image height
 * @param[in]  format           image format
 * @param[in]  type             image type name
 * @param[out] errorOut         error info
 * @return                      created TiVo image parameters on success, NULL in case of error
 **/
TiVoImageDBImageParams
TiVoImageDBImageParamsCreate(unsigned int width, unsigned int height, TiVoImageDBImageFormat format, SvString type, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // TIVO_IMAGE_DB_H_

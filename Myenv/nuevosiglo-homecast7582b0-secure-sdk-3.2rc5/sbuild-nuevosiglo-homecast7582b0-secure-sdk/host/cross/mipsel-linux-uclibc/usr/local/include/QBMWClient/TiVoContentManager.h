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

#ifndef TIVO_CONTENT_MANAGER_H_
#define TIVO_CONTENT_MANAGER_H_

/**
 * @file TiVoContentManager.h
 * @brief Content manager for TiVo
 **/

/**
 * @defgroup TiVoContentManager
 * @ingroup QBMWClient
 * @{
 **/
#include <QBMWClient/QBMWClientContentManager.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

/**
 * TiVo content manager type.
 **/
typedef enum {
    TiVoContentManagerType_default = 0,
    TiVoContentManagerType_discoveryBar
} TiVoContentManagerType;

/**
 * TiVo content manager parameters.
 **/
typedef struct TiVoContentManagerParams_s {
    /// content manager params
    struct QBMWClientContentManagerParams_s super_;
    /// content manager type
    TiVoContentManagerType type;
    /// parameters of required images
    SvArray imageParamsSet;
    /// content requests desc. path
    SvString requestsPath;
} *TiVoContentManagerParams;

/**
 * Get runtime type identification object representing
 * TiVo content manager parameters.
 * @return TiVo content manager parameters object
 **/
SvType
TiVoContentManagerParams_getType(void);


/**
 * TiVo content manager class.
 *
 * This class is responsible for providing content metadata
 * through creating and handling appropriate content providers.
 *
 * @class TiVoContentManager
 * @extends ContentManager
 **/
typedef struct TiVoContentManager_ *TiVoContentManager;

/**
* Get runtime type identification object representing TiVoContentManager class.
*
* @return TiVo content manager class
**/
SvType TiVoContentManager_getType(void);

/**
 * @}
 **/

#endif // TIVO_CONTENT_MANAGER_H_

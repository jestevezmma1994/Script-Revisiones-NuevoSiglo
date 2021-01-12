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

#ifndef SV_RB_FONT_H_
#define SV_RB_FONT_H_

/**
 * @file SvRBFont.h
 * @brief Resource manager font class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURI.h>
#include <QBResourceManager/SvRBObject.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvRBFont Resource manager font class
 * @ingroup QBResourceManagerLibrary
 * @{
 *
 * A class representing CAGE font managed as the resource manager object.
 *
 * @image html SvRBFont.png
 **/

/**
 * Resource manager font class.
 * @class SvRBFont SvRBFont.h <QBResourceManager/SvRBFont.h>
 * @extends SvRBObject
 **/
typedef struct SvRBFont_ *SvRBFont;


/**
 * Get runtime type identification object representing type
 * of resource manager font class.
 *
 * @relates SvRBFont
 *
 * @return resource manager font class
 **/
extern SvType SvRBFont_getType(void);

/**
 * Create new resource manager font object.
 *
 * This method creates new resource manager object representing CAGE font.
 * Created object can then be registered in the resource manager service, which
 * keeps track of its registered resources (represented as resource manager
 * objects) and their usage, freeing them when appropriate.
 *
 * @memberof SvRBFont
 *
 * @param[in] URI       URI of the source file, can be a local path
 * @return              created object or @c NULL in case of error
 **/
extern SvRBFont SvRBFontCreateWithURI(SvURI URI);

/**
 * Create new resource manager font object.
 *
 * This is a convenience wrapper for SvRBFontCreateWithURI().
 *
 * @memberof SvRBFont
 *
 * @param[in] URI       URI or a local path of the source file
 * @return              created object or @c NULL in case of error
 **/
extern SvRBFont SvRBFontCreate(const char *URI);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

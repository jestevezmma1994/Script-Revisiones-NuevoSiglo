/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CAGE_FT_FONT_H_
#define CAGE_FT_FONT_H_

/**
 * @file SvFTFont.h FreeType font provider class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvFTFont FreeType font provider class
 * @ingroup CAGE_TextRendering
 * @{
 *
 * SvFTFont is an implementation of the @ref SvFontProvider
 * based on the open-source FreeType library.
 *
 * @image html SvFTFont.png
 **/


/**
 * FreeType font provider class.
 * @class SvFTFont
 * @extends SvObject
 **/
typedef struct SvFTFont_ *SvFTFont;


/**
 * Return the runtime type identification object
 * of the SvFTFont class.
 *
 * @return FreeType font provider class
 **/
extern SvType
SvFTFont_getType(void);

/**
 * Initialize FreeType font provider instance.
 *
 * @memberof SvFTFont
 *
 * @param[in] self      FreeType font provider handle
 * @param[in] filePath  path to a font file supported by FreeType library
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvFTFont
SvFTFontInit(SvFTFont self,
             const char *filePath,
             SvErrorInfo *errorOut);

/**
 * Initialize FreeType font provider instance with
 * font loaded into memory.
 *
 * @memberof SvFTFont
 *
 * @param[in] self      FreeType font provider handle
 * @param[in] memBuffer buffer containing loaded font file
 * @param[in] fileURI   URI of a font file loaded into @a memBuffer
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvFTFont
SvFTFontInitWithMemory(SvFTFont self,
                       SvData memBuffer,
                       const char *fileURI,
                       SvErrorInfo *errorOut);

/**
 * Check if loaded font file is a TrueType font file.
 *
 * @param[in] self      FreeType font provider handle
 * @return              @c true for TrueType font file, otherwise @c false
 **/
extern bool
SvFTFontIsTrueType(SvFTFont self);

/**
 * Get FreeType font face handle.
 *
 * @param[in] self      FreeType font provider handle
 * @return              FreeType FT_Face handle
 **/
extern void *
SvFTFontGetFTFace(SvFTFont self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

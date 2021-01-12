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

#ifndef SV_GFX_CONFIG_H_
#define SV_GFX_CONFIG_H_

/**
 * @file SvGfxConfig.h Declaration of data types describing GFX engine configuration
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CAGE/Core/SvColor.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup SvGfxEngine
 * @{
 **/

/**
 * Configuration of the GFX engine.
 **/
typedef struct SvGfxConfig_t {
    /// current configuration of the graphics canvas
    struct {
        /// graphics canvas width in pixel
        unsigned int width;
        /// graphics canvas height in pixel
        unsigned int height;
        /// aspect ratio of the graphics canvas
        QBAspectRatio aspectRatio;
        /// deprecated information, do not use
        unsigned int frameLag;
    } gfx;

    /**
     * Some HALs support 'partial rendering', that is, the upper layers
     * may send just a diff between previous and next frame and HAL
     * will be able to draw correct frame. If this flag is set to true
     * it means that HAL is unable to do that and forces callers to
     * provide full redraw information.
     **/
    bool forceFullRedraw;
} SvGfxConfig;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

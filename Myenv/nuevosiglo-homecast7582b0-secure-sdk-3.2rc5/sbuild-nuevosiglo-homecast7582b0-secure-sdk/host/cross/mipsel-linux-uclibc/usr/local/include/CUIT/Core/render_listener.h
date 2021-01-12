/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUIT_RENDER_LISTENER_H_
#define CUIT_RENDER_LISTENER_H_

/**
 * @file SMP/CUIT/render_listener.h Render listener interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvAppRenderListener Render listener interface
 * @ingroup SvApplication
 * @{
 *
 * An interface for listeners that want to be notified about render state.
 **/

/**
 * Render listener interface.
 **/
typedef const struct SvAppRenderListener_ {
    /**
     * This method is called when CUIT starts rendering next frame.
     *
     * @param[in] self_ listener handle
     **/
    void (*onRender)(SvObject self_);
} *SvAppRenderListener;


/**
 * Get runtime type identification object representing SvAppRenderListener interface.
 *
 * @return SvAppRenderListener interface object
 **/
extern SvInterface
SvAppRenderListener_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

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

#ifndef TV_OSD_PLUGIN_INTERFACE
#define TV_OSD_PLUGIN_INTERFACE

/**
 * @file TVOSDPluginInterface.h OSD plugin interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvInterface.h>
#include <CUIT/Core/widget.h>

/**
 * TVOSDPlugin interface. Plugins allow to introduce modifications to original OSD.
 *
 * @interface TVOSDPlugin
 **/
typedef struct TVOSDPlugin_ {
    /**
     * Start the TVOSDPlugin.
     *
     * @param[in] self      self handle
     * @param[in] osd       widget which performs function of OSD
     **/
    void (*start)(SvObject self, SvWidget osd);

    /**
     * Stop the the TVOSDPlugin.
     *
     * @param[in] self      self handle
     **/
    void (*stop)(SvObject self);
} *TVOSDPlugin;

/**
 * Get runtime type identification object representing
 * TVOSDPlugin interface.
 *
 * @return TVOSDPlugin interface object
 **/

extern SvInterface TVOSDPlugin_getInterface(void);

/**
 * @}
 **/

#endif

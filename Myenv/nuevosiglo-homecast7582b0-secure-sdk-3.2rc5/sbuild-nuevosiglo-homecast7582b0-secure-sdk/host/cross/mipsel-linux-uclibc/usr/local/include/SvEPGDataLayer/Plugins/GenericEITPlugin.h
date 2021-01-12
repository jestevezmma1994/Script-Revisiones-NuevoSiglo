/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef GENERIC_EIT_PLUGIN_H_
#define GENERIC_EIT_PLUGIN_H_

/**
 * @file GenericEITPlugin.h
 * @brief Generic EIT plugin class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup GenericEITPlugin Generic EIT plugin
 * @ingroup SvEPGPlugins
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>


/**
 * @defgroup GenericEITPlugin_ Generic EIT plugin class
 * @ingroup GenericEITPlugin
 * @{
 **/

/**
 * Generic EIT plugin class.
 **/
typedef struct GenericEITPlugin_ *GenericEITPlugin;


/**
 * Get runtime type identification object
 * representing Generic EIT plugin class.
 **/
extern SvType
GenericEITPlugin_getType(void);

/**
 * Initialize GenericEITPlugin instance.
 *
 * @param[in] self      Generic EIT plugin handle
 * @param[in] mapper    handle to an object implementing @ref SvEITChannelMapper
 * @param[in] tunerCnt  number of tuners to use
 * @param[out] errorOut error info
 * @return              self or NULL in case of error
 **/
extern GenericEITPlugin
GenericEITPluginInit(GenericEITPlugin self,
                     SvGenericObject mapper,
                     unsigned int tunerCnt,
                     SvErrorInfo *errorOut);

/**
 * Set muxes to use by GenericEITPlugin.
 *
 * @param[in] self      Generic EIT plugin handle
 * @param[in] muxes     handle to an array of muxes
 * @param[in] errorOut  error info handle
 **/
extern void
GenericEITPluginSetAllowedMuxes(GenericEITPlugin self, SvArray muxes, SvErrorInfo* errorOut);

/**
 * @}
 **/


#endif

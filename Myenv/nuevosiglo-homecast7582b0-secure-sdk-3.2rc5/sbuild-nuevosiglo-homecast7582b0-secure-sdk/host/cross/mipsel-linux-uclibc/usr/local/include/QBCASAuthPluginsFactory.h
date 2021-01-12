/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCAS_AUTH_PLUGINS_FACTORY_H_

#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCASAuthPluginsFactory.h QBCASAuthPluginsFactory interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCASAuthPluginsFactory QBCASAuthPluginsFactory
 * Interface of CAS specific data authentication plugins factory.
 *
 * @ingroup DRM
 * @{
 **/

/**
 * QBCASAuthPluginsFactory interface of CAS specific data authentication plugins factory
 */
typedef struct QBCASAuthPluginsFactory_s
{
    /**
     * Create metafile authentication plugin.
     * @param[in] self_ handle to object which implements @link QBCASAuthPluginsFactory @endlink
     * @return          handle to metafile authentication plugin
     */
    SvObject (*createMetaFileAuthPlugin) (SvObject self_);
} *QBCASAuthPluginsFactory;

/**
 * Get runtime type identification object representing
 * QBCASAuthPluginsFactory interface.
 *
 * @return QBCASAuthPluginsFactory interface.
 **/
SvInterface QBCASAuthPluginsFactory_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // QBCAS_AUTH_PLUGINS_FACTORY_H_


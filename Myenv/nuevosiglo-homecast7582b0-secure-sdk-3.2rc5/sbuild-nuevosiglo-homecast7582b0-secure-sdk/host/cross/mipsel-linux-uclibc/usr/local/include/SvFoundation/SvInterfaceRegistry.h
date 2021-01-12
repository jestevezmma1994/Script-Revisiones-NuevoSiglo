/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SvFoundation_SvInterfaceRegistry_h
#define SvFoundation_SvInterfaceRegistry_h

/**
 * @file SvInterfaceRegistry.h Global registry of interfaces
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvImmutableArray.h>


/**
 * @defgroup SvInterfaceRegistry Global registry of interfaces
 * @ingroup SvFoundation
 * @{
 **/

/**
 * Find an interface with given name in global registry.
 *
 * @param[in] name interface name
 * @return interface with @a name, @c NULL if not found
 **/
extern SvInterface
SvInterfaceRegistryFindInterface(SvString name);

/**
 * Find a type with given name in global registry.
 *
 * @param[in] name type name
 * @return type with @a name, @c NULL if not found
 **/
extern SvType
SvInterfaceRegistryFindType(SvString name);

/**
 * Create an array of all registered interfaces.
 *
 * @qb_allocator
 *
 * @param[out] errorOut error info
 * @return created array of all known interfaces
 **/
extern SvImmutableArray
SvInterfaceRegistryCreateInterfacesList(SvErrorInfo *errorOut);

/**
 * Create an array of all registered types.
 *
 * @qb_allocator
 *
 * @param[out] errorOut error info
 * @return created array of all known types
 **/
extern SvImmutableArray
SvInterfaceRegistryCreateTypesList(SvErrorInfo *errorOut);

/**
 * Install debug hooks for all new instances of all types.
 *
 * This method will install debug hooks in all currently registered
 * types and setup the registry so that they will be automatically
 * registered in all types created afterwards. As a result, full debugging
 * (including object lifecycle tracking) will be enabled.
 *
 * @param[in] debugHooks object debug hooks
 **/
extern void
SvInterfaceRegistryInstallObjectDebugHooks(const struct SvObjectDebugHooks_ *const debugHooks);

/**
 * @}
 **/


#endif

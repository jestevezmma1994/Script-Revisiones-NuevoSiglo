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

#ifndef SV_COMMON_DEFS_H_
#define SV_COMMON_DEFS_H_

/**
 * @file SvCommonDefs.h Commonly used macros
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvCommonDefs Commonly used macros
 * @ingroup SvCore
 * @{
 *
 * Macrodefinitions for specifying visibility and other attributes
 * of functions and variables.
 **/


/**
 * use 'SvLocal' instead of 'static' qualifier for private methods, because
 * it can be redefined to '' when compiling code for unit tests, that should
 * have access to all internal methods
 **/
#ifndef SvLocal
# define SvLocal static
#endif

/**
 * use 'SvHidden' to mark methods that are called from other compilation
 * units, but are not needed outside the library &mdash; this prevents linker
 * from exporting their names as global symbols in shared library, which
 * keeps namespace cleaner and makes libraries load a little faster
 **/
#ifndef SvHidden
# define SvHidden __attribute__ ((visibility ("hidden")))
#endif

/**
 * use 'SvExport' to mark methods that should be visible when using
 * "-fvisibility=hidden"
 **/
#ifndef SvExport
# define SvExport __attribute__ ((visibility ("default")))
#endif

/**
 * use 'SvUnused' to mark variables, functions and function arguments
 * that in some circumstances can be unused
 **/
#ifndef SvUnused
# define SvUnused __attribute__ ((unused))
#endif


#ifndef SvConstructor
# define SvConstructor __attribute__ ((constructor))
#endif

#ifndef SvDestructor
# define SvDestructor __attribute__ ((destructor))
#endif


#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)


#if SV_LOG_LEVEL > 0
#define SvDebugString(x) (x)
#else
#define SvDebugString(x) ""
#endif


/**
 * @}
 **/

#endif

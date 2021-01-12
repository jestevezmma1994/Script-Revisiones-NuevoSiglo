/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
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

#ifndef SV_CORE_ERROR_DOMAIN_H_
#define SV_CORE_ERROR_DOMAIN_H_

/**
 * @file SvCoreErrorDomain.h SvCore error domain
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvCoreErrorDomain SvCore error domain
 * @ingroup SvCore
 * @{
 *
 * Error domain defining common error codes used by entire media platform.
 **/

/**
 * The SvCore error domain.
 **/
#define SvCoreErrorDomain "com.sentivision.SvCore"

/**
 * SvCore error domain error codes.
 *
 * This set of error codes represents the most generic, abstract error conditions
 * that can be reported by various components. In essence they are similar to
 * errno error codes with one important difference of being well-defined in this file.
 *
 * @note There is a special error domain called SvErrnoDomain that wraps the
 * POSIX errno error codes from the underlying operating system. You should use
 * them when wrapping codes returned from operating system libraries. In other
 * cases you should use this core domain or define your own.
 **/
enum SvCoreError {
    /** @cond */
    SvCoreError_reserved = 0,
    /** @endcond */

    /**
     * Invalid argument error code.
     *
     * This error code can be used whenever function/method argument is invalid
     * in some way. It corresponds to the EINVAL errno code.
     *
     * @note In previous versions of this library this value was called
     * SvInvalidArgumentError
     *
     * @since 1.6
     **/
    SvCoreError_invalidArgument = 1,

    /**
     * Invalid index error code.
     *
     * This error code can be used whenever function/method argument uses some kind
     * of index with invalid value. It can be useful to differentiate from
     * invalid argument error when all the arguments are valid except for the
     * index that is off-bounds.
     *
     * @note In previous versions of this library this value was called
     * SvInvalidIndexError
     *
     * @since 1.6
     **/
    SvCoreError_invalidIndex,

    /**
     * Invalid state error code.
     *
     * This error code can be useful whenever function/method success depends
     * on some internal state and that state was incorrect for the requested
     * operation.
     *
     * @note In previous versions of this library this value was called
     * SvInvalidStateError
     *
     * @since 1.6
     **/
    SvCoreError_invalidState,

    /**
     * Not-found error code.
     *
     * This error code can be useful for function/method that performs some
     * lookups and the requested item cannot be found.
     *
     * @note In previous versions of this library this value was called
     * SvNotFoundError
     *
     * @since 1.6
     **/
    SvCoreError_notFound,

    /**
     * No memory error.
     *
     * This error code can be useful for functions that perform some kind of
     * memory allocation. It is currently used by SvCore'a allocator code.
     *
     * @since 1.6
     **/
    SvCoreError_noMemory,

    /** @cond */
    SvCoreError_last = SvCoreError_noMemory
    /** @endcond */
};

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

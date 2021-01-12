/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SvFoundation_SvComparator_h
#define SvFoundation_SvComparator_h

/**
 * @file SvComparator.h Utilities for comparing objects
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvComparator Object comparator interface
 * @ingroup SvFoundationInterfaces
 * @{
 **/


/**
 * A function for comparing two objects.
 *
 * @param[in] prv       opaque pointer to function's private data
 * @param[in] objectA   a handle to the first object
 * @param[in] objectB   a handle to the second object
 * @return              less than @c 0 if @a objectA < @a objectB,
 *                      @c 0 if @a objectA = @a objectB,
 *                      more than @c 0 if @a objectA > @a objectB
 **/
typedef int (*SvObjectCompareFn)(void *prv,
                                 SvObject objectA,
                                 SvObject objectB);


/**
 * SvComparator interface.
 **/
typedef const struct SvComparator_ {
    /**
     * Compare two objects.
     *
     * @param[in] self_     an object implementing @ref SvComparator
     * @param[in] objectA   a handle to the first object
     * @param[in] objectB   a handle to the second object
     * @return              less than @c 0 if @a objectA < @a objectB,
     *                      @c 0 if @a objectA = @a objectB,
     *                      more than @c 0 if @a objectA > @a objectB
     **/
    int (*compare)(SvObject self_,
                   SvObject objectA,
                   SvObject objectB);
} *SvComparator;


/**
 * Get runtime type identification object representing SvComparator interface.
 *
 * @return SvComparator interface object
 **/
extern SvInterface
SvComparator_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

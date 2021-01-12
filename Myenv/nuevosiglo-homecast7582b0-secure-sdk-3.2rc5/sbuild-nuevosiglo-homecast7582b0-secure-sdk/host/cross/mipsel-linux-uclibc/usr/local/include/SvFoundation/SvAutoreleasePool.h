/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SvFoundation_SvAutoreleasePool_h
#define SvFoundation_SvAutoreleasePool_h

/**
 * @file SvAutoreleasePool.h Autorelease pool class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvAutoreleasePool Autorelease pool class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * Autorelease pool class.
 * @class SvAutoreleasePool
 * @extends SvObject
 **/
typedef struct SvAutoreleasePool_ *SvAutoreleasePool;


/**
 * Get runtime type identification object representing SvAutoreleasePool class.
 *
 * @since 1.10
 *
 * @return autorelease pool class
 **/
extern SvType
SvAutoreleasePool_getType(void);

/**
 * Create an autorelease pool.
 *
 * This method creates new autorelease pool.
 * Each created autorelease pool becomes the current pool for current thread.
 * The previous pool is remembered and will be restored as soon as this pool
 * is destroyed.
 *
 * This scheme allows for efficient and simple memory management by
 * following a simple pattern:
 *  - create and destroy (release) one autorelease pool in your main loop
 *    (to get rid of temporary objects allocated during one iteration),
 *  - create another pool around your whole loop and carefully monitor what
 *    gets inserted into that pool: those objects will likely live as long as
 *    the whole application,
 *  - additionally, you may want to create a pool around some memory intensive
 *    operations as an optimalization (but always check that it indeed improves
 *    something -- a two-layer model is sufficient for all of our code).
 *
 * @memberof SvAutoreleasePool
 *
 * @param[out] errorOut error info
 * @return new empty autorelease pool, @c NULL in case of error
 **/
extern SvAutoreleasePool
SvAutoreleasePoolCreate(SvErrorInfo *errorOut);

/**
 * Remove objects from the autorelease pool.
 *
 * @memberof SvAutoreleasePool
 *
 * @param[in] self a handle to the autorelease pool
 * @return number of removed objects
 **/
extern size_t
SvAutoreleasePoolDrain(SvAutoreleasePool self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SvFoundation_SvDebug_h
#define SvFoundation_SvDebug_h

/**
 * @file SvDebug.h SvFoundation debugging support
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvFoundationDebug Debugging support
 * @ingroup SvFoundation
 * @{
 *
 * Facilities for run-time inspection of type system internal mechanisms.
 **/

struct SvWeakReferencesChainElement_;


/**
 * Object debug hooks.
 **/
struct SvObjectDebugHooks_ {
    /// private data handle passed to debug hooks
    void *prv;

    /**
     * Notify that object have been retained (its reference counter
     * have been incremented.)
     *
     * @note This function will also be called when reference counter
     * is initially set to @c 1, i.e. when object is created.
     *
     * @param[in] prv       private data handle
     * @param[in] obj       handle to retained object
     **/
    void (*onRetain)(void *prv, SvObject obj);

    /**
     * Notify that object is to be released (its reference counter
     * will be decremented.)
     *
     * @param[in] prv       private data handle
     * @param[in] obj       handle to released object
     **/
    void (*onRelease)(void *prv, SvObject obj);

    /**
     * Notify that new weak reference to the object have been added.
     *
     * @param[in] prv       private data handle
     * @param[in] obj       handle to the object
     * @param[in] elem      new element of the weak references chain
     **/
    void (*onWeakReferenceAdd)(void *prv, SvObject obj,
                               const struct SvWeakReferencesChainElement_ *elem);

    /**
     * Notify that the weak reference to the object is to be removed.
     *
     * @param[in] prv       private data handle
     * @param[in] obj       handle to the object
     * @param[in] elem      element of the weak references chain
     *                      that will be removed
     **/
    void (*onWeakReferenceRemove)(void *prv, SvObject obj,
                                  const struct SvWeakReferencesChainElement_ *elem);

    /**
     * Notify that the object still exists when global library destructor is running.
     *
     * @param[in] prv       private data handle
     * @param[in] obj       handle to the object
     **/
    void (*onShutdown)(void *prv, SvObject obj);
};


/**
 * Check if debugging is enabled globally.
 *
 * @return @c true if debugging is enabled, @c false otherwise
 **/
extern bool SvFoundationDebugIsEnabled(void);

/**
 * Return the number of living objects allocated so far.
 *
 * @return number of living objects
 **/
extern size_t SvFoundationDebugGetObjectsCount(void);

/**
 * Translate address of a memory block returned by memory allocator
 * into object residing in this block.
 *
 * @since 1.13
 *
 * @param[in] block         memory block containing object
 * @return                  handle to the object, @c NULL if not found
 **/
extern SvObject SvFoundationDebugLocateObject(void *block);

/**
 * @}
 **/


/**
 * @addtogroup SvType
 * @{
 **/

/**
 * Install debug hooks for all new type instances.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @param[in] debugHooks object debug hooks
 **/
extern void
SvTypeInstallObjectDebugHooks(SvType self,
                              const struct SvObjectDebugHooks_ *const debugHooks);

/**
 * Create an array of all type instances.
 *
 * @note You must enable debugging for this type for this method to work.
 *
 * @memberof SvType
 * @qb_allocator
 *
 * @param[in] self type handle
 * @param[out] errorOut error info
 * @return created array of all instances, @c NULL in case of error
 **/
extern SvArray
SvTypeCreateInstancesList(SvType self,
                          SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

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

#ifndef SvFoundation_SvObject_h
#define SvFoundation_SvObject_h

/**
 * @file SvObject.h Base object class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <QBOutputStream.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvObject Base object class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/


// forward declarations
struct SvObjectDebugHooks_;
struct SvWeakReferencesChainElement_;

/**
 * Get runtime type identification object representing SvObject class.
 *
 * @return base object class
 **/
extern SvType
SvObject_getType(void) __attribute__ ((const));

/**
 * Return a pointer to the extra space area of the given object.
 *
 * This area is of the same size that was specified during object creation.
 *
 * @param[in] self reference to an object.
 * @returns a pointer to the extra space.
 *
 * @memberof SvObject
 * @since 1.0
 **/
extern void *
SvObjectGetExtraSpacePointer(SvObject self);

/**
 * Allocate block of memory to be used internally by an object.
 *
 * This function allocates memory in such way that memory allocator
 * accounts this allocation to the same memory pool as the base object.
 *
 * To free this block of memory, use:
@code
  #include <SvCore/SvAllocator.h>

  SvAllocatorDeallocate(SvTypeGetAllocator(SvObjectGetType(self)), ptr);
@endcode
 *
 * @since 1.12
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @param[in] size requested amount of memory in bytes
 * @param[in] alignment memory address alignment, @c 0 to use default
 *         alignment required for C language
 * @param[out] errorOut error info
 * @return a pointer to newly allocated memory or @c NULL if
 *         the allocation could not be performed
 **/
extern void *
SvObjectAllocateStorage(SvObject self,
                        size_t size,
                        size_t alignment,
                        SvErrorInfo *errorOut);

/**
 * Get the type of an object.
 *
 * @since 1.2
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @return object type
 **/
extern SvType
SvObjectGetType(SvObject self);

/**
 * Get the super type of an object.
 *
 * @since 1.2
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @return type of object's super class
 **/
extern SvType
SvObjectGetSuperType(SvObject self);

/**
 * Get the name of the type associated with an object.
 *
 * @since 1.2
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @return name of object's type
 **/
extern const char *
SvObjectGetTypeName(SvObject self);

/**
 * Get the retain count of an object.
 *
 * @since 1.2
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @return object's retain count
 **/
extern size_t
SvObjectGetRetainCount(SvObject self);

/**
 * Check if an object is an instance of the given type or it's sub-type.
 *
 * @since 1.0
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @param[in] type type handle
 * @return @c true if object is an instance of @a type,
 *         @c false if not or when @a self or @a type is @c NULL
 **/
extern bool
SvObjectIsInstanceOf(SvObject self,
                     SvType type);

/**
 * Check if object is an implementation of the given interface.
 *
 * @since 1.12
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @param[in] iface interface handle
 * @return @c true if object is an implementation of @a iface,
 *         @c false if not or when @a self or @a iface is @c NULL
 **/
extern bool
SvObjectIsImplementationOf(SvObject self,
                           SvInterface iface);

/**
 * Get the methods table of an interface implemented by object's type.
 *
 * @see SvInvokeInterface().
 *
 * @since 1.12
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @param[in] iface interface handle
 * @return handle to the methods table, @c NULL if @a iface is not implemented
 **/
extern const void *
SvObjectGetVTable(SvObject self,
                  SvInterface iface);

/**
 * Get the methods table of an interface implemented by object's type.
 *
 * @see SvInvokeInterface().
 *
 * @since 1.12
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @param[in] interfaceID unique identifier of an interface
 * @return handle to the methods table, @c NULL if this interface is not implemented
 **/
extern const void *
SvObjectGetVTableByID(SvObject self,
                      size_t interfaceID);

/**
 * Install debug hooks for tracking object's life cycle.
 *
 * @memberof SvObject
 * @private
 *
 * @param[in] self object handle
 * @param[in] debugHooks object debug hooks
 * @param[out] errorOut error info
 **/
extern void
SvObjectInstallDebugHooks(SvObject self,
                          const struct SvObjectDebugHooks_ *const debugHooks,
                          SvErrorInfo *errorOut);

/**
 * Retain a reference to an object.
 *
 * @since 1.3
 * @memberof SvObject
 *
 * @note Do not use this function directly. Use SVRETAIN() or SVTESTRETAIN() instead.
 *
 * @param[in] self a non-NULL reference to any object
 * @return reference to the retained object
 **/
extern void *
SvObjectRetain(SvObject self);

/** @cond */
static inline void * __attribute__ ((always_inline))
SvObjectTestRetain(SvObject self)
{
    return self ? SvObjectRetain(self) : NULL;
}
/** @endcond */

/**
 * Release a reference to an object.
 *
 * @since 1.3
 * @memberof SvObject
 *
 * @note Do not use this function directly. Use SVRELEASE() or SVTESTRELEASE() instead.
 *
 * @param[in] self a non-NULL reference to any object
 **/
extern void
SvObjectRelease(SvObject self);

/** @cond */
static inline void __attribute__ ((always_inline))
SvObjectTestRelease(SvObject self)
{
    if (self)
        SvObjectRelease(self);
}
/** @endcond */

/**
 * Retain a reference to an object.
 *
 * This macro simply calls SvObjectRetain(), casting the argument to
 * SvObject. This is the preferred way of retaining a non-NULL object.
 * Do not use SvObjectRetain() directly.
 *
 * @since 1.1
 *
 * @param[in] object non-NULL object handle
 * @return reference to the retained object
 **/
#define SVRETAIN(object) ((void *) SvObjectRetain((SvObject)(object)))

/**
 * Retain a (potentially @c NULL) reference to an object.
 *
 * This macro simply calls SVRETAIN() when the object is not @c NULL.
 * This is the preferred way of retaining a (possibly @c NULL) object.
 * Do not use SvObjectRetain() directly.
 *
 * @since 1.1
 *
 * @param[in] object object handle
 * @return reference to the retained object or @c NULL if the object was @c NULL
 **/
#define SVTESTRETAIN(object) ((void *) SvObjectTestRetain((SvObject)(object)))

/**
 * Release a reference to an object.
 *
 * This macro simply calls SvObjectRelease(), casting the argument to
 * SvObject. This is the preferred way of releasing a non-NULL object.
 * Do not use SvObjectRelease() directly.
 *
 * @param[in] object non-NULL object handle
 *
 * @since 1.1
 **/
#define SVRELEASE(object) SvObjectRelease((SvObject)(object))

/**
 * Release a (potentially @c NULL) reference to an object.
 *
 * This macro simply calls SVRELEASE() when the object is not @c NULL.
 * This is the preferred way of releasing a (possible @c NULL) object.
 * Do not use SvObjectRelease() directly.
 *
 * @since 1.1
 *
 * @param[in] object object handle
 **/
#define SVTESTRELEASE(object) SvObjectTestRelease((SvObject)(object))

/**
 * Add given object to the default autorelease pool.
 *
 * This method passes the ownership of an object from caller
 * to the default autorelease pool.
 *
 * @since 1.0
 * @memberof SvObject
 *
 * @note Do not use this function directly; use SVAUTORELEASE()
 *       or SVTESTAUTORELEASE() instead.
 *
 * @param[in] self object handle
 * @return @a self
 **/
extern void *
SvObjectAutorelease(SvObject self);

/** @cond */
static inline void *
SvObjectTestAutorelease(SvObject self)
{
    return self ? SvObjectAutorelease(self) : NULL;
}
/** @endcond */

/**
 * Autorelease a reference to an object.
 *
 * This macro simply calls SvObjectAutorelease(), casting the argument to
 * SvObject. This macro is the preferred way of autoreleasing a non-NULL object.
 * Do not use SvObjectAutorelease() directly.
 *
 * @since 1.1
 *
 * @param[in] object object handle
 **/
#define SVAUTORELEASE(object) ((void *) SvObjectAutorelease((SvObject)(object)))

/**
 * Autorelease a (potentially @c NULL) reference to an object.
 *
 * This macro simply calls SVAUTORELEASE() when the object is not @c NULL.
 *
 * @since 1.1
 *
 * @param[in] object object handle
 **/
#define SVTESTAUTORELEASE(object) ((void *) SvObjectTestAutorelease((SvObject)(object)))

/**
 * Add new weak reference to an object.
 *
 * @since 1.12
 * @memberof SvObject
 * @private
 *
 * @param[in] self object handle
 * @param[in] chainElement new element of the list of weak references for an object
 *
 * @internal
 **/
extern void
SvObjectAddWeakReference(SvObject self,
                         struct SvWeakReferencesChainElement_ *chainElement);

/**
 * Remove existing weak reference to an object.
 *
 * @since 1.12
 * @memberof SvObject
 * @private
 *
 * @param[in] self object handle
 * @param[in] chainElement element of the list of weak references for an object
 *
 * @internal
 **/
extern void
SvObjectRemoveWeakReference(SvObject self,
                            struct SvWeakReferencesChainElement_ *chainElement);

/**
 * Check if two objects are equal.
 *
 * @since 1.0
 * @memberof SvObject
 *
 * @param[in] self object handle
 * @param[in] other object to compare to
 * @return @c true if objects are equal or both are @c NULL, @c false otherwise
 **/
extern bool
SvObjectEquals(SvObject self,
               SvObject other);

/**
 * Calculate the hash of an object.
 *
 * @since 1.0
 * @memberof SvObject
 *
 * @param[in] self non-NULL object handle
 * @return object's hash value
 **/
extern unsigned int
SvObjectHash(SvObject self);

/**
 * Create a deep copy of the specified object.
 *
 * The class of that object must support copying by implementing
 * virtual copy() method. If the class of that object
 * does not implement this method then a @ref SvFoundationError_interfaceNotImplemented
 * error is returned.
 *
 * The value returned by SvObjectCopy() is implicitly retained by the caller
 * that is responsible for releasing it.
 *
 * The copy does not always have to create duplicate object. Instances of
 * immutable data types usually just SVRETAIN() themselves instead of copying.
 *
 * @since 1.2
 * @memberof SvObject
 *
 * @param[in] self a non-null reference to an object
 * @param[out] errorOut error info
 * @return copy of @a self or @c NULL in case of any error
 **/
extern SvObject
SvObjectCopy(SvObject self,
             SvErrorInfo *errorOut);

/**
 * Write object description to output stream.
 *
 * @since 1.12
 * @memberof SvObject
 *
 * @param[in] self a non-null reference to an object
 * @param[in] outputStream output stream to write to
 **/
extern void
SvObjectDescribe(SvObject self,
                 QBOutputStream outputStream);

/** @cond */
extern SvObject
SvObjectAllocateAnonymous_(const char *srcFileName,
                           unsigned int srcFileLine,
                           size_t instanceSize,
                           SvType superType,
                           SvErrorInfo *errorOut,
                           ...);
/** @endcond */

/**
 * Allocate an anonymous object (an instance of anonymous type)
 * implementing a list of interfaces.
 *
 * This method creates an anonymous type and allocates single instance
 * of this type. Type will be automatically destroyed when not needed anymore.
 *
 * The list of implemented interfaces consists of pairs of pointers,
 * refer to SvTypeCreateManaged() for more information.
 *
 * @since 1.12
 *
 * @param[in] instanceSize requested size of an instance
 * @param[in] superType handle to a super type
 * @param[out] errorOut error info
 * @return allocated anonymous instance or @c NULL in case of error
 *
 * @hideinitializer
 **/
#if SV_LOG_LEVEL > 0
# define SvObjectAllocateAnonymous(instanceSize, superType, errorOut, ...) SvObjectAllocateAnonymous_(__FILE__, __LINE__, (instanceSize), (superType), (errorOut), ## __VA_ARGS__)
#else
# define SvObjectAllocateAnonymous(instanceSize, superType, errorOut, ...) SvObjectAllocateAnonymous_(NULL, 0, (instanceSize), (superType), (errorOut), ## __VA_ARGS__)
#endif

/**
 * Call a virtual method on an object.
 *
 * This macro expands to code that will call virtual method of the given object.
 *
 * @note The @a obj argument is evaluated twice!
 *
 * @since 1.12
 *
 * @param[in] type type containing @a method
 * @param[in] obj handle to an object implementing virtual @a method of @a type
 * @param[in] method method to be called
 **/
#define SvInvokeVirtual(type, obj, method, ...) \
    ((type ## VTable) SvObjectGetVTableByID((SvObject)(obj), 0))->method((obj), ## __VA_ARGS__)

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

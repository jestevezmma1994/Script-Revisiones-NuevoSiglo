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

#ifndef SvFoundation_SvType_h
#define SvFoundation_SvType_h

/**
 * @file SvType.h Type class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvAllocator.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvType Type class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * Get runtime type identification object representing SvType class.
 *
 * @return type class
 **/
extern SvType
SvType_getType(void);


/**
 * Create new type at runtime.
 *
 * @memberof SvType
 * @since 1.9
 *
 * @param[in] name unique name for the new type
 * @param[in] instanceSize size of a single instance of this class
 * @param[in] superType handle to a super type
 * @param[out] errorOut error info
 * @return new type or @c NULL in case of error
 **/
extern SvType
SvTypeCreate(const char *name,
             size_t instanceSize,
             SvType superType,
             SvErrorInfo *errorOut);

/**
 * Create new type implementing a list of interfaces.
 *
 * This method creates new type and stores the pointer to its master
 * reference. This master reference will be automatically set to @c NULL
 * when type is destroyed.
 *
 * The list of implemented interfaces consists of pairs of pointers,
 * first one to the implemented interface object, second one to the table
 * of virtual methods for this interface; see @ref DeclaringNewClass.
 *
 * @memberof SvType
 * @since 1.11
 *
 * @param[in] name unique name for the new type
 * @param[in] instanceSize size of a single instance of this class
 * @param[in] superType handle to a super type
 * @param[out] masterReference pointer to a variable that will be used to keep
 *             the reference to this type; it will be automatically updated
 *             when type is destroyed
 * @return new type or @c NULL in case of error
 **/
extern SvType
SvTypeCreateManaged(const char *name,
                    size_t instanceSize,
                    SvType superType,
                    volatile SvType *masterReference,
                    ...);

/**
 * Create new type implementing virtual methods and a list of interfaces.
 *
 * This method is an extended version of SvTypeCreateManaged().
 *
 * @memberof SvType
 * @since 1.12
 *
 * @param[in] name unique name for the new type
 * @param[in] instanceSize size of a single instance of this class
 * @param[in] superType handle to a super type
 * @param[in] virtualTableSize size in bytes of the virtual methods table
 * @param[in] virtualTable virtual methods table
 * @param[out] masterReference pointer to a variable that will be used to keep
 *             the reference to this type; it will be automatically updated
 *             when type is destroyed
 * @return new type or @c NULL in case of error
 **/
extern SvType
SvTypeCreateVirtual(const char *name,
                    size_t instanceSize,
                    SvType superType,
                    size_t virtualTableSize,
                    const void *virtualTable,
                    volatile SvType *masterReference,
                    ...);

/**
 * Allocate an object of given type.
 *
 * @memberof SvType
 * @since 1.7
 * @qb_allocator
 *
 * @param[in] self type handle
 * @param[out] errorOut error info
 * @return allocated instance of the type or @c NULL in case of error
 **/
extern SvObject
SvTypeAllocateInstance(SvType self,
                       SvErrorInfo *errorOut);

/**
 * Allocate an object of given type with some extra space.
 *
 * @memberof SvType
 * @since 1.5
 * @qb_allocator
 *
 * @param[in] self type handle
 * @param[in] extraSpace size of extra space (in bytes) that extends
 *                       beyond base instance size
 * @param[out] errorOut error info
 * @return allocated instance of the type or @c NULL in case of error
 **/
extern SvObject
SvTypeAllocateInstanceWithExtraSpace(SvType self,
                                     size_t extraSpace,
                                     SvErrorInfo *errorOut);

/**
 * Get type name.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @return name of the type
 **/
extern const char *
SvTypeGetName(SvType self);

/**
 * Get type's super type.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @return super type handle, @c NULL if none
 **/
extern SvType
SvTypeGetSuperType(SvType self);

/**
 * Get type's instance size.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @return size of type's instances in bytes
 **/
extern size_t
SvTypeGetInstanceSize(SvType self);

/**
 * Get the methods table of an interface implemented by a type.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @param[in] ID interface ID, @c 0 to get virtual methods table
 * @return handle to the methods table, @c NULL if interface
 *         with given @a ID is not implemented
 **/
extern void *
SvTypeGetVTable(SvType self,
                long int ID);

/**
 * Get handle to the memory allocator used by a type.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @return handle to a memory allocator
 **/
extern SvAllocator
SvTypeGetAllocator(SvType self);

/**
 * Mark type as final.
 *
 * This method marks type as final. It's not possible to inherit
 * from a final type.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @param[out] errorOut error info
 **/
extern void
SvTypeSetFinal(SvType self,
               SvErrorInfo *errorOut);

/**
 * Check if type is final.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @return @c true if type is final
 **/
extern bool
SvTypeIsFinal(SvType self);

/**
 * Mark type instances as immutable.
 *
 * This method will cause all instances of a type to be treated
 * as immutable, for example they will have default trivial copy() method.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @param[out] errorOut error info
 **/
extern void
SvTypeSetImmutable(SvType self,
                   SvErrorInfo *errorOut);

/**
 * Check if type is a supertype of another type.
 *
 * @memberof SvType
 * @since 1.12
 *
 * @param[in] self type handle
 * @param[in] other type handle to check
 * @return @c true if @a self is a supertype of @a other
 **/
extern bool
SvTypeIsSuperTypeOf(SvType self,
                    SvType other);

/**
 * Check if type instances are immutable.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @return @c true if type instances are immutable
 **/
extern bool
SvTypeIsImmutable(SvType self);

/**
 * Mark type as abstract.
 *
 * This method will mark type as abstract, i.e. it won't be possible to create
 * any instances of this type.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @param[out] errorOut error info
 **/
extern void
SvTypeSetAbstract(SvType self,
                  SvErrorInfo *errorOut);

/**
 * Check if type is abstract.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @return @c true if type is abstract
 **/
extern bool
SvTypeIsAbstract(SvType self);

/**
 * Check if type has object debugging enabled.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 * @return @c true if new instances are created with debugging enabled
 **/
extern bool
SvTypeHasDebuggingEnabled(SvType self);

/**
 * Enable debugging for type instances.
 *
 * @memberof SvType
 *
 * @param[in] self type handle
 **/
extern void
SvTypeEnableDebugging(SvType self);

/**
 * @brief Return number of live objects.
 *
 * @memberof SvType
 * @since 1.12
 *
 * @param [in] self type handle
 *
 * @return number of live objects.
 **/
extern long int
SvTypeGetInstancesCount(SvType self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif

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

#ifndef SvFoundation_SvInterface_h
#define SvFoundation_SvInterface_h

/**
 * @file SvInterface.h Interface class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h> // for ssize_t
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvInterface Interface class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * SvInterface class internals.
 * @private
 * @since 1.9
 **/
struct SvInterface_ {
    /// super class
    struct SvObject_ super_;

    /**
     * Name of the interface.
     *
     * @private
     * @since 1.9
     **/
    SvString name;

    /**
     * Unique interface identifier.
     * All interfaces share some data in the runtime.
     * This identifier is the key for getting that data.
     *
     * @private
     * @since 1.9
     **/
    long int interfaceID;

    /**
     * Base interface.
     *
     * Unlike Java we can have only one, Java performs lookup on methods, we
     * perform lookup on entire interfaces. Thus we have interface granularity
     * and cannot support multiple inherited interfaces because there would be
     * no way to correctly place fields with method pointers.
     *
     * @private
     * @since 1.9
     **/
    SvInterface superInterface;

    /**
     * Number of sizeof(void *) methods in the method table of conforming
     * implementations.
     *
     * @private
     * @since 1.9
     **/
    size_t methodsCount;

    /**
     * Location of a master reference to the managed interface.
     *
     * @private
     * @since 1.11
     **/
    volatile SvInterface *masterReference;
};


/**
 * Get runtime type identification object representing SvInterface class.
 *
 * @since 1.9
 *
 * @return interface class
 **/
extern SvType
SvInterface_getType(void);

/**
 * Initialize an interface.
 *
 * @memberof SvInterface
 * @since 1.11
 *
 * @param[in] self interface object handle
 * @param[in] name interface name
 * @param[in] methodTableSize size in bytes of the virtual methods table
 * @param[in] superInterface base interface or @c NULL
 * @param[out] errorOut error info
 * @return @a self or @c NULL in case of error
 **/
extern SvInterface
SvInterfaceInit(SvInterface self,
                const char *name,
                size_t methodTableSize,
                SvInterface superInterface,
                SvErrorInfo *errorOut);

/**
 * Create an interface at runtime,
 * updating and remembering master reference to the created object.
 *
 * @memberof SvInterface
 * @since 1.11
 *
 * @param[in] name interface name
 * @param[in] methodTableSize size in bytes of the virtual methods table
 * @param[in] superInterface base interface or @c NULL
 * @param[out] masterReference master reference to created interface
 * @param[out] errorOut error info
 * @return created interface or @c NULL in case of error
 **/
extern SvInterface
SvInterfaceCreateManaged(const char *name,
                         size_t methodTableSize,
                         SvInterface superInterface,
                         volatile SvInterface *masterReference,
                         SvErrorInfo *errorOut);

/**
 * Get interface name.
 *
 * @memberof SvInterface
 * @since 1.12
 *
 * @param[in] self interface handle
 * @return name of the interface, @c NULL in case of error
 **/
extern const char *
SvInterfaceGetName(SvInterface self);

/**
 * Get interface's super interface.
 *
 * @memberof SvInterface
 * @since 1.12
 *
 * @param[in] self interface handle
 * @return super interface handle, @c NULL if none
 **/
extern SvInterface
SvInterfaceGetSuperInterface(SvInterface self);

/**
 * Get interface's unique identifier.
 *
 * @memberof SvInterface
 * @since 1.12
 *
 * @param[in] self interface handle
 * @return unique identifier of the interface, @c -1 in case of error
 **/
extern long int
SvInterfaceGetID(SvInterface self);

/**
 * Get number of methods in interface.
 *
 * @memberof SvInterface
 * @since 1.12
 *
 * @param[in] self interface handle
 * @return number of methods of interface, @c -1 in case of error
 **/
extern ssize_t
SvInterfaceGetMethodsCount(SvInterface self);

/**
 * Call an interface method on an object.
 *
 * This macro expands to code that will call interface method of the given object.
 *
 * @note The @a obj argument is evaluated twice!
 *
 * @since 1.9
 *
 * @param[in] interface interface containing @a method
 * @param[in] obj handle to an object implementing @a interface
 * @param[in] method method to be called
 **/
#define SvInvokeInterface(interface, obj, method, ...) \
    ((interface) SvObjectGetVTable((SvObject)(obj), interface ## _getInterface()))->method((SvObject)(obj), ## __VA_ARGS__)

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

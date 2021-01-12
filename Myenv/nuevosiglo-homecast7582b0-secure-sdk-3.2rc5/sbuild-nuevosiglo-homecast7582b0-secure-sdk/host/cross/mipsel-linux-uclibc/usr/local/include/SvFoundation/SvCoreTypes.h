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

#ifndef SvFoundation_SvCoreTypes_h
#define SvFoundation_SvCoreTypes_h

/**
 * @file SvCoreTypes.h SvFoundation core types
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvByteOrder.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Base object class.
 * @class SvObject
 * @ingroup SvObject
 **/
typedef struct SvObject_ *SvObject;

/**
 * Interface class.
 * @class SvInterface
 * @extends SvObject
 * @ingroup SvInterface
 **/
typedef struct SvInterface_ *SvInterface;

/**
 * Type class.
 * @class SvType
 * @extends SvInterface
 * @ingroup SvType
 **/
typedef struct SvType_ *SvType;

/**
 * String class.
 * @class SvString
 * @extends SvObject
 * @ingroup SvString
 **/
typedef struct SvString_ *SvString;


/**
 * SvObject class internals.
 * @ingroup SvObject
 **/
struct SvObject_ {
    /** The type information of an object.
     *
     * @private
     *
     * @note Never use this field directly. Use SvObjectGetType() to read it.
     * Use SvObjectGetSuperType() to get the super type. Use
     * SvObjectGetTypeName() to extract the class name.
     *
     * @since 1.0
     **/
    SvType type;

    /**
     * The retain count of an object.
     *
     * @private
     *
     * @note Never use this field directly. Use SVRETAIN() and SVRELEASE() to
     * manipulate the counter. Use SvObjectGetRetainCount() to obtain the
     * current value.
     *
     * @since 1.0
     **/
    long int retainCount;

    /*
     * This conditional memory layout is needed to satisfy one requirement:
     * offset of ::magic in SvObject must be the same as offset of the least
     * significant byte of ::debugHooks in SvObjectPrivateData. Address of
     * ::debugHooks has its 2 least significant bits cleared, SV_OBJECT_MAGIC_VALUE
     * has them set. This property is used by SvFoundationDebugLocateObject() to
     * differentiate between objects with private data block and objects without it.
     */
#if SV_BYTE_ORDER == SV_LITTLE_ENDIAN
    /**
     * Object magic value, must be set to #SV_OBJECT_MAGIC_VALUE.
     *
     * @private
     * @since 1.13
     **/
    uint8_t magic;

    /**
     * Object flags.
     *
     * @private
     * @since 1.12
     **/
    uint8_t flags;

    /**
     * Padding.
     * @private
     **/
    uint8_t pad[sizeof(void *) - 2];
#else
    /**
     * Padding.
     * @private
     **/
    uint8_t pad[sizeof(void *) - 2];

    /**
     * Object flags.
     *
     * @private
     * @since 1.12
     **/
    uint8_t flags;

    /**
     * Object magic value, must be set to #SV_OBJECT_MAGIC_VALUE.
     *
     * @private
     * @since 1.13
     **/
    uint8_t magic;
#endif

    /**
     * Dynamically allocated list of weak references.
     *
     * @private
     *
     * @note Never use this field directly. It has complex memory ownership
     * policy associated with it and should never be needed directly.
     *
     * @since 1.2
     **/
    void *refList;
} __attribute__ ((aligned(8)));

/**
 * Magic value that marks SvObject in memory.
 * @ingroup SvObject
 * @see SvObject_::magic
 **/
#define SV_OBJECT_MAGIC_VALUE   0xd3U

/**
 * SvObject virtual methods table.
 * @ingroup SvObject
 **/
typedef struct SvObjectVTable_ {
    /**
     * SvObject destructor method.
     *
     * @since 1.0
     **/
    void (*destroy)(void *self);

    /**
     * SvObject equals virtual method.
     *
     * @since 1.0
     **/
    bool (*equals)(void *self, void *other);

    /**
     * SvObject hash virtual method.
     *
     * @since 1.0
     **/
    unsigned int (*hash)(void *self);

    /**
     * SvObject deep copy virtual method.
     *
     * @since 1.2
     **/
    void *(*copy)(void *self, SvErrorInfo *errorOut);

    /**
     * SvObject describe virtual method.
     *
     * @since 1.12
     **/
    void (*describe)(void *self, void *outputStream);
} *SvObjectVTable;


// compatibility macros
/** @cond */
#define SvGenericObject SvObject
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif

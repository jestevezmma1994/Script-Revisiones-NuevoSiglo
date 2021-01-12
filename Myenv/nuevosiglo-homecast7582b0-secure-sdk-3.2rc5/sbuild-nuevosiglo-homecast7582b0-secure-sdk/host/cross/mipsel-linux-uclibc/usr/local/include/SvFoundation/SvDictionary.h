/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SvFoundation_SvDictionary_h
#define SvFoundation_SvDictionary_h

/**
 * @file SvDictionary.h Dictionary interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDictionary Dictionary interface
 * @ingroup SvFoundationInterfaces
 * @{
 **/

/**
 * SvDictionary interface.
 *
 * @since 1.11.7
 **/
typedef const struct SvDictionary_ {
    /**
     * Get number of key:value pairs in the collection.
     *
     * @param[in] self_ an object implementing @ref SvDictionary
     * @return number of pairs
     **/
    size_t (*getPairsCount)(SvObject self_);

    /**
     * Get object associated with given key.
     *
     * @param[in] self_ an object implementing @ref SvDictionary
     * @param[in] key key that the value is mapped to
     * @return a handle to the value associated with @a key, @c NULL if unavailable
     **/
    SvObject (*lookup)(SvObject self_,
                       SvObject key);

    /**
     * Insert value with given key.
     *
     * @param[in] self_ an object implementing @ref SvDictionary
     * @param[in] key key to associate @a obj with
     * @param[in] value handle to an object to be added to collection,
     * @param[out] errorOut error info
     **/
    void (*insert)(SvObject self_,
                   SvObject key,
                   SvObject value,
                   SvErrorInfo *errorOut);

    /**
     * Remove object associated with given key.
     *
     * @param[in] self_ an object implementing @ref SvDictionary
     * @param[in] key key that the value is mapped to
     * @param[out] errorOut error info
     **/
    void (*remove)(SvObject self_,
                   SvObject key,
                   SvErrorInfo *errorOut);

    /**
     * Get iterator over all keys.
     *
     * @param[in] self_ an object implementing @ref SvDictionary
     * @return iterator
     **/
    SvIterator (*getKeysIterator)(SvObject self_);
} *SvDictionary;


/**
 * Get runtime type identification object representing
 * SvDictionary interface.
 *
 * @since 1.11.7
 *
 * @return SvDictionary interface object
 **/
extern SvInterface
SvDictionary_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

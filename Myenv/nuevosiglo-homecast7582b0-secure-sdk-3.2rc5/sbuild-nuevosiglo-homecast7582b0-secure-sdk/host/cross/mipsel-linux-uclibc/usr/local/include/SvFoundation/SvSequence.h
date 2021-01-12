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

#ifndef SvFoundation_SvSequence_h
#define SvFoundation_SvSequence_h

/**
 * @file SvSequence.h Sequence interface
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
 * @defgroup SvSequence Sequence interface
 * @ingroup SvFoundationInterfaces
 * @{
 **/

/**
 * SvSequence interface.
 *
 * @since 1.11.7
 **/
typedef const struct SvSequence_ {
    /**
     * Get number of objects in the collection.
     *
     * @param[in] self_ an object implementing @ref SvSequence
     * @return number of objects
     **/
    size_t (*getObjectsCount)(SvObject self_);

    /**
     * Get object at specified index.
     *
     * @param[in] self_ an object implementing @ref SvSequence
     * @param[in] idx index of the object to retrieve, starting with @c 0
     * @return a handle to the object at @a idx, @c NULL if unavailable
     **/
    SvObject (*getAtIndex)(SvObject self_,
                           size_t idx);

    /**
     * Set object at given index replacing previous one.
     *
     * @param[in] self_ an object implementing @ref SvSequence
     * @param[in] idx requested index of @a obj in the collection
     * @param[in] obj handle to an object to be added to collection
     * @param[out] errorOut error info
     **/
    void (*setAtIndex)(SvObject self_,
                       size_t idx,
                       SvObject obj,
                       SvErrorInfo *errorOut);

    /**
     * Insert object at given index.
     *
     * This method inserts new object to the collection. Unlike
     * SvSequence::setAtIndex() this method won't replace object
     * that previously occupied @a idx, but move all objects at
     * indices >= @a idx to make place for new object.
     *
     * @param[in] self_ an object implementing @ref SvSequence
     * @param[in] idx requested index of @a obj in the collection
     * @param[in] obj handle to an object to be added to collection
     * @param[out] errorOut error info
     **/
    void (*insertAtIndex)(SvObject self_,
                          size_t idx,
                          SvObject obj,
                          SvErrorInfo *errorOut);

    /**
     * Remove object at given index.
     *
     * This method removes an object from the collection.
     * Objects at indices >= @a idx are moved to fill the gap.
     *
     * @param[in] self_ an object implementing @ref SvSequence
     * @param[in] idx index of object to be removed
     * @param[out] errorOut error info
     **/
    void (*removeAtIndex)(SvObject self_,
                          size_t idx,
                          SvErrorInfo *errorOut);

    /**
     * Get iterator over entire collection.
     *
     * @param[in] self_ an object implementing @ref SvSequence
     * @return iterator
     **/
    SvIterator (*getIterator)(SvObject self_);
} *SvSequence;


/**
 * Get runtime type identification object representing
 * SvSequence interface.
 *
 * @since 1.11.7
 *
 * @return SvSequence interface object
 **/
extern SvInterface
SvSequence_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_SAFE_ITERATOR_H_
#define QB_SAFE_ITERATOR_H_

/**
 * @file QBSafeIterator.h Iterator for collections that keep weak references to objects
 **/

#include <stddef.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBSafeIterator Safe iterator type
 * @ingroup QBDataModel3
 * @{
 *
 * An iterator for collections that keep only weak references to objects.
 **/

/**
 * Safe iterator type.
 **/
typedef struct QBSafeIterator_ QBSafeIterator;

/**
 * Safe iterator methods.
 * @internal
 **/
typedef struct QBSafeIteratorMethods_ {
    /**
     * Get object pointed to by the iterator and advance iterator's position.
     *
     * @param[in] subject       an object iterated over
     * @param[in,out] iterator  iterator handle
     * @return                  a retained handle to the next object, @c NULL if unavailable
     **/
    SvObject (*getNext)(SvObject subject,
                        QBSafeIterator *iterator);
} QBSafeIteratorMethods;

/**
 * QBSafeIterator type internals.
 * @internal
 **/
struct QBSafeIterator_ {
    SvObject subject;
    QBSafeIteratorMethods methods;
    union {
        size_t z;
        void *ptr;
    } auxA, auxB;
};


/**
 * Get object pointed to by the iterator and advance iterator's position.
 *
 * @note This method passes ownership of the returned object to the caller, caller is responsible for releasing it.
 *
 * @param[in,out] iterator      iterator handle
 * @return                      a retained handle to the next object, @c NULL if unavailable
 **/
static inline SvObject
QBSafeIteratorGetNext(QBSafeIterator *iterator)
{
    return iterator->methods.getNext(iterator->subject, iterator);
}

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

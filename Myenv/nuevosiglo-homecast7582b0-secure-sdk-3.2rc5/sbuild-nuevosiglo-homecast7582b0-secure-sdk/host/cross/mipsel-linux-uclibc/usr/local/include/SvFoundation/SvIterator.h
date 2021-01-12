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

#ifndef SvFoundation_SvIterator_h
#define SvFoundation_SvIterator_h

/**
 * @file SvIterator.h Collection iterator
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvIterator Collection iterator
 * @ingroup SvFoundationCollections
 * @{
 **/

/**
 * SvIterator type.
 **/
typedef struct SvIterator_ SvIterator;

/**
 * SvIterator methods.
 * @internal
 **/
struct SvIteratorMethods {
    /**
     * Check if iterator can return next object.
     *
     * @param[in] subject an object iterated over
     * @param[in] iterator iterator handle
     * @return @c true if next object is available, @c false if not
     **/
    bool (*hasNext)(SvObject subject,
                    const SvIterator *iterator);

    /**
     * Get object pointed to by the iterator without advancing
     * iterator's position.
     *
     * @param[in] subject an object iterated over
     * @param[in] iterator iterator handle
     * @return a handle to the next object, @c NULL if unavailable
     **/
    SvObject (*peekNext)(SvObject subject,
                         const SvIterator *iterator);

    /**
     * Get object pointed to by the iterator and advance iterator's position.
     *
     * @param[in] subject an object iterated over
     * @param[in,out] iterator iterator handle
     * @return a handle to the next object, @c NULL if unavailable
     **/
    SvObject (*getNext)(SvObject subject,
                        SvIterator *iterator);
};


/**
 * SvIterator type internals.
 * @internal
 **/
struct SvIterator_ {
    /// an object iterated over
    SvObject subject;
    /// iterator implementation
    const struct SvIteratorMethods *methods;
    union {
        /// value as a boolean
        bool b;
        /// value as an integer
        int i;
        /// value as index or size
        size_t z;
        /// value as a pointer
        void *ptr;
    } auxA, ///< current position/value of the iterator, first part
      auxB; ///< current position/value of the iterator, second part
};


/**
 * Compare two iterators.
 *
 * @note This method will work correctly only if compared
 *       iterators are for the same object!
 *
 * @param[in] iteratorA first iterator handle
 * @param[in] iteratorB second iterator handle
 * @return @c true if iterators are equal (point to the same position), @c false if not
 **/
static inline bool
SvIteratorEquals(const SvIterator *const iteratorA,
                 const SvIterator *const iteratorB)
{
    return (iteratorA->auxA.z == iteratorB->auxA.z) && (iteratorA->auxB.z == iteratorB->auxB.z);
}

/**
 * Check if iterator can return next object.
 *
 * @param[in] iterator iterator handle
 * @return @c true if next object is available, @c false if not
 **/
static inline bool
SvIteratorHasNext(const SvIterator *iterator)
{
    return iterator->methods->hasNext(iterator->subject, iterator);
}

/**
 * Get object pointed to by the iterator without advancing
 * iterator's position.
 *
 * @param[in] iterator iterator handle
 * @return a handle to the next object, @c NULL if unavailable
 **/
static inline SvObject
SvIteratorPeekNext(const SvIterator *iterator)
{
    return iterator->methods->peekNext(iterator->subject, iterator);
}

/**
 * Get object pointed to by the iterator and advance iterator's position.
 *
 * @param[in,out] iterator iterator handle
 * @return a handle to the next object, @c NULL if unavailable
 **/
static inline SvObject
SvIteratorGetNext(SvIterator *iterator)
{
    return iterator->methods->getNext(iterator->subject, iterator);
}

/**
 * Return an iterator that will never point to any object.
 * @return void iterator
 **/
extern SvIterator
SvGetVoidIterator(void);

/** @cond */
static inline SvIterator
SvVoidIterator(void)
{
    return SvGetVoidIterator();
}
/** @endcond */

/**
 * Create automatic iterator over a collection of objects.
 *
 * This method returns an iterator over a collection of objects
 * passed as arguments. Objects are stored in a temporary immutable
 * array, that is autoreleased before this method returns. Format
 * of @a typeSpec argument is the same as for
 * SvImmutableArrayCreateWithTypedValues() method.
 *
 * It can be used for creating complex data structures:
@code
    SvIterator i = SvGetAutoIterator("ss @i @d @? @s @vs @@ @v@ @*",
        "_type",                "channel",
        SVSTRING("ID"),         7,
        SVSTRING("rating"),     4.5,
        SVSTRING("isAdult"),    false,
        SVSTRING("contentURI"), "rtsp://127.0.0.1/vod/2",
        SVSTRING("cookie"),     "fe7beac7aa94d9b7ffbae57536ca1252",
        SVSTRING("name"),       SVSTRING("CNN"),
        SVSTRING("something"),  SVAUTORELEASE(SvStringCreate("abc", NULL)),
        SVSTRING("_rawPtr"),    NULL
    );
    SvHashTable channel = SvHashTableCreateWithKeyValuePairs(&i, NULL);
@endcode
 *
 * @param[in] typeSpec a string specifying types of arguments
 * @return an iterator over collection of objects
 **/
extern SvIterator
SvGetAutoIterator(const char *typeSpec,
                  ...);

/** @cond */
extern SvIterator
SvAutoIterator(const char *typeSpec,
               ...);
/** @endcond */

/**
 * Create automatic iterator over a collection of objects.
 *
 * This method is another version of SvGetAutoIterator().
 *
 * @param[in] typeSpec a string specifying types of arguments
 * @param[in] args list of arguments
 * @return an iterator over collection of objects
 **/
extern SvIterator
SvGetAutoIteratorV(const char *typeSpec,
                   va_list args);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

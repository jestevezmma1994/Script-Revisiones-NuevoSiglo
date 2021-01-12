/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SvFoundation_SvHashTable_h
#define SvFoundation_SvHashTable_h

/**
 * @file SvHashTable.h Hash table class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvHashTable Hash table class
 * @ingroup SvFoundationCollections
 * @{
 **/


/**
 * Hash table class.
 * @class SvHashTable
 * @extends SvObject
 **/
typedef struct SvHashTable_ *SvHashTable;


/**
 * Get runtime type identification object representing SvHashTable class.
 *
 * @return hash table class
 **/
extern SvType
SvHashTable_getType(void);


/**
 * Initialize a hash table.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @param[in] size number of unique hash values
 * @param[out] errorOut error info
 * @return @a self or @c NULL in case of error
 */
extern SvHashTable
SvHashTableInit(SvHashTable self,
                size_t size,
                SvErrorInfo *errorOut);

/**
 * Create a hash table.
 *
 * @memberof SvHashTable
 *
 * @param[in] size number of unique hash values
 * @param[out] errorOut error info
 * @return initialized empty hash table or @c NULL in case of error
 */
extern SvHashTable
SvHashTableCreate(size_t size,
                  SvErrorInfo *errorOut);

/**
 * Create a hash table with keys and values fetched by the @a iterator.
 *
 * @memberof SvHashTable
 *
 * @param[in,out] iterator iterator for fetching (key, value) pairs
 *                         (must return an even number of objects)
 * @param[out] errorOut error info
 * @return new hash table filled with objects from @a iterator
 *         or @c NULL in case of error
 */
extern SvHashTable
SvHashTableCreateWithKeyValuePairs(SvIterator *iterator,
                                   SvErrorInfo *errorOut);

/**
 * Create a hash table with keys and values fetched by two iterators.
 *
 * @memberof SvHashTable
 *
 * @param[in,out] keysIterator   iterator for fetching keys
 * @param[in,out] valuesIterator iterator for fetching values
 *                               (must return at least as much values
 *                               as @a keysIterator)
 * @param[out] errorOut error info
 * @return new hash table filled with objects from @a iterator
 *         or @c NULL in case of error
 */
extern SvHashTable
SvHashTableCreateWithKeysAndValues(SvIterator *keysIterator,
                                   SvIterator *valuesIterator,
                                   SvErrorInfo *errorOut);

/**
 * Create an array with all the keys in an array.
 *
 * @memberof SvHashTable
 * @qb_allocator
 *
 * @param[in] self hash table handle
 * @return array of all keys
 */
extern SvArray
SvHashTableCreateKeysList(SvHashTable self);

/**
 * Create an array with all the values in an array.
 *
 * @memberof SvHashTable
 * @qb_allocator
 *
 * @param[in] self hash table handle
 * @return array of all values
 */
extern SvArray
SvHashTableCreateValuesList(SvHashTable self);

/** @cond */
static inline SvArray
SvHashTableConstructAllValues(SvHashTable self)
{
    return SvHashTableCreateValuesList(self);
}
/** @endcond */

/**
 * Get size of hash table.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @return number of unique hash values
 */
extern size_t
SvHashTableGetSize(SvHashTable self);

/** @cond */
static inline size_t
SvHashTableSize(SvHashTable self)
{
    return SvHashTableGetSize(self);
}
/** @endcond */

/**
 * Get number of key-value pairs stored in hash table.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @return number of key-value pairs in hash table
 */
extern size_t
SvHashTableGetCount(SvHashTable self);

/** @cond */
static inline size_t
SvHashTableCount(SvHashTable self)
{
    return SvHashTableGetCount(self);
}
/** @endcond */

/**
 * Return the element with given key.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @param[in] key element's key
 * @return element's value or @c NULL
 */
extern SvObject
SvHashTableFind(SvHashTable self,
                SvObject key);

/**
 * Insert new element to the hash table, possibly replacing an old one
 * with the same key.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @param[in] key element's key
 * @param[in] value element's value
 */
extern void
SvHashTableInsert(SvHashTable self,
                  SvObject key,
                  SvObject value);

/**
 * Insert new elements to the hash table, fetching keys and values from
 * two different iterators.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @param[in] keysIterator iterator returning keys
 * @param[in] valuesIterator iterator returning values
 */
extern void
SvHashTableInsertObjects(SvHashTable self,
                         SvIterator *keysIterator,
                         SvIterator *valuesIterator);

/**
 * Remove the element with given key.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @param[in] key element's key
 * @return @a key if found, @c NULL if not found
 */
extern SvObject
SvHashTableRemove(SvHashTable self,
                  SvObject key);

/**
 * Remove all elements from the hash table.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 */
extern void
SvHashTableRemoveAllObjects(SvHashTable self);

/**
 * Resize hash table.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @param[in] newSize new size
 */
extern void
SvHashTableResize(SvHashTable self,
                  size_t newSize);

/**
 * Move all (key, value) pairs from given hash table.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @param[in] source source of elements, will be empty afterwards
 */
extern void
SvHashTableMove(SvHashTable self,
                SvHashTable source);

/**
 * Get an iterator for iterating over hash table values.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @return the iterator
 **/
extern SvIterator
SvHashTableGetValuesIterator(SvHashTable self);

/** @cond */
static inline SvIterator
SvHashTableValuesIterator(SvHashTable self)
{
    return SvHashTableGetValuesIterator(self);
}
/** @endcond */

/**
 * Get an iterator for iterating over hash table keys.
 *
 * @memberof SvHashTable
 *
 * @param[in] self hash table handle
 * @return the iterator
 **/
extern SvIterator
SvHashTableGetKeysIterator(SvHashTable self);

/** @cond */
static inline SvIterator
SvHashTableKeysIterator(SvHashTable self)
{
    return SvHashTableGetKeysIterator(self);
}
/** @endcond */

/**
 * Get a value corresponding to a key that will be returned as the next one
 * by an iterator over hash table keys, without advancing the iterator.
 *
 * @memberof SvHashTable
 * @since 1.12.1
 *
 * @param[in] self hash table handle
 * @param[in] iterator iterator over hash table keys
 * @return value for the next key, @c NULL if not available
 **/
extern SvObject
SvHashTablePeekValue(SvHashTable self,
                     const SvIterator *iterator);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

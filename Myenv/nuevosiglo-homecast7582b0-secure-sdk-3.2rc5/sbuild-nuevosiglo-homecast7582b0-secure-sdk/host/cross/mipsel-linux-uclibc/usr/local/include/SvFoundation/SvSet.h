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

#ifndef SvFoundation_SvSet_h
#define SvFoundation_SvSet_h

/**
 * @file SvSet.h Set class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvSet Set class
 * @ingroup SvFoundationCollections
 * @{
 **/

/**
 * Set class.
 * @class SvSet
 * @extends SvObject
 **/
typedef struct SvSet_ *SvSet;


/**
 * Get runtime type identification object representing SvSet class.
 *
 * @return set class
 **/
extern SvType
SvSet_getType(void);

/**
 * Create a set.
 *
 * @memberof SvSet
 *
 * @param[in] size approximate number of elements
 * @param[out] errorOut error info
 * @return new empty set or @c NULL in case of error
 */
extern SvSet
SvSetCreate(size_t size,
            SvErrorInfo *errorOut);

/**
 * Create universe (a set containing all possible values).
 *
 * @memberof SvSet
 *
 * @param[in] size storage size hint
 * @param[out] errorOut error info
 * @return new universal set or @c NULL in case of error
 */
extern SvSet
SvSetCreateUniversal(size_t size,
                     SvErrorInfo *errorOut);

/**
 * Check if a set contains given element.
 *
 * @memberof SvSet
 *
 * @param[in] self a set
 * @param[in] element an element to be checked for
 * @return @c true if set contains the @a element
 **/
extern bool
SvSetContainsElement(SvSet self,
                     SvObject element);

/**
 * Get number of elements in a set.
 *
 * This method returns number of elements in a set, if the set
 * is finite. For infinite sets, error is thrown.
 *
 * @memberof SvSet
 *
 * @param[in] self a set
 * @param[out] errorOut error info
 * @return number of elements, @c -1 in case of error
 **/
extern ssize_t
SvSetGetCardinality(SvSet self,
                    SvErrorInfo *errorOut);

/** @cond */
static inline ssize_t
SvSetCardinality(SvSet self,
                 SvErrorInfo *errorOut)
{
    return SvSetGetCardinality(self, errorOut);
}
/** @endcond */

/**
 * Check if given set is empty.
 *
 * @memberof SvSet
 *
 * @param[in] self a set
 * @return true if set is empty
 **/
extern bool
SvSetIsEmpty(SvSet self);

/**
 * Check if given set is a universal set.
 *
 * @memberof SvSet
 *
 * @param[in] self a set
 * @return true if set is finite
 **/
extern bool
SvSetIsUniversal(SvSet self);

/**
 * Check if given set has finite number of elements.
 *
 * @memberof SvSet
 *
 * @param[in] self a set
 * @return true if set is finite
 **/
extern bool
SvSetIsFinite(SvSet self);

/**
 * Add a single element to the set.
 *
 * @memberof SvSet
 *
 * @param[in] self a set
 * @param[in] element element to be added to the set
 **/
extern void
SvSetAddElement(SvSet self,
                SvObject element);

/**
 * Remove a single element from the set.
 *
 * @memberof SvSet
 *
 * @param[in] self a set
 * @param[in] element element to be removed from the set
 **/
extern void
SvSetRemoveElement(SvSet self,
                   SvObject element);

/**
 * Remove all elements from the set.
 *
 * @memberof SvSet
 *
 * @param[in] self a set
 **/
extern void
SvSetRemoveAllElements(SvSet self);

/**
 * Create an array containing all elements of a set.
 *
 * This method creates an array containing all elements
 * of a set. This method will fail if called on infinite set.
 *
 * @memberof SvSet
 * @qb_allocator
 *
 * @param[in] self a set
 * @param[out] errorOut error info
 * @return array with all elements of a set
 **/
extern SvArray
SvSetCreateElementsList(SvSet self,
                        SvErrorInfo *errorOut);

/**
 * Create a set being the union of two other sets.
 *
 * This method returns new set containing the union (the sum)
 * of two other sets, @a setA and @a setB.
 *
 * @memberof SvSet
 *
 * @param[in] setA first set
 * @param[in] setB second set
 * @param[out] errorOut error info
 * @return a set being the union of @a setA and @a setB
 **/
extern SvSet
SvSetCreateUnion(SvSet setA,
                 SvSet setB,
                 SvErrorInfo *errorOut);

/**
 * Create a set being the intersection of two other sets.
 *
 * This method returns new set containing the intersection
 * of two other sets, @a setA and @a setB.
 *
 * @memberof SvSet
 *
 * @param[in] setA first set
 * @param[in] setB second set
 * @param[out] errorOut error info
 * @return a set being the intersection of @a setA and @a setB
 **/
extern SvSet
SvSetCreateIntersection(SvSet setA,
                        SvSet setB,
                        SvErrorInfo *errorOut);

/**
 * Create a set being the relative complement of two other sets.
 *
 * This method returns new set being the relative complement of the
 * @a setA in @a setB (also known as set theoretic difference of @a setB
 * and @a setA.)
 *
 * @memberof SvSet
 *
 * @param[in] setA first set
 * @param[in] setB second set
 * @param[out] errorOut error info
 * @return a set being the complement of @a setA in @a setB
 **/
extern SvSet
SvSetCreateRelativeComplement(SvSet setA,
                              SvSet setB,
                              SvErrorInfo *errorOut);

/**
 * Create a set being an absolute complement of a given set.
 *
 * This method returns new set being the absolute complement of the @a set.
 *
 * @memberof SvSet
 *
 * @param[in] set a set
 * @param[out] errorOut error info
 * @return a set being the absolute complement of @a set
 **/
extern SvSet
SvSetCreateComplement(SvSet set,
                      SvErrorInfo *errorOut);

/**
 * Get an iterator for iterating over set elements.
 *
 * @memberof SvSet
 *
 * @param[in] self set handle
 * @param[out] errorOut error info
 * @return the iterator
 **/
extern SvIterator
SvSetGetIterator(SvSet self,
                 SvErrorInfo *errorOut);

/** @cond */
static inline SvIterator
SvSetIterator(SvSet self,
              SvErrorInfo *errorOut)
{
    return SvSetGetIterator(self, errorOut);
}
/** @endcond */

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

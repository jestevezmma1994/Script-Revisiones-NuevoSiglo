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

#ifndef SV_DATA_BUCKET_H_
#define SV_DATA_BUCKET_H_

/**
 * @file SvDataBucket.h Data bucket class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvArray.h>
#include <SvDataBucket2/SvDBObject.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDataBucket Data bucket class
 * @ingroup SvDataBucket2
 * @{
 *
 * Data bucket is a container that keeps track of all registered instances
 * of data bucket objects.
 **/

/**
 * Data bucket class.
 * @class SvDataBucket
 * @extends SvObject
 **/
typedef struct SvDataBucket_ *SvDataBucket;


/**
 * Create new data bucket object.
 *
 * @memberof SvDataBucket
 *
 * @param[out] errorOut error info
 * @return              created object, @c NULL in case of error
 **/
extern SvDataBucket
SvDataBucketCreate(SvErrorInfo *errorOut);

/**
 * Get number of objects held by the data bucket.
 *
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @return              number of objects
 **/
extern size_t
SvDataBucketGetObjectCount(SvDataBucket self);

/**
 * Register custom object constructor in the data bucket.
 *
 * This method registers constuctor for objects of type @a typeName.
 * When parser finds new object, that have a custom constructor,
 * it will be used instead of default constructor. Default constructor
 * creates objects of type SvDBRawObject.
 *
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @param[in] typeName  object type name
 * @param[in] ctor      constructor for objects of type @a typeName
 * @param[out] errorOut error info
 **/
extern void
SvDataBucketRegisterObjectConstructor(SvDataBucket self,
                                      SvString typeName,
                                      SvDBObjectConstructorFunc ctor,
                                      SvErrorInfo *errorOut);

/**
 * Register custom object merger function in the data bucket.
 *
 * This method registers merger for objects of type @a typeName.
 * Default merger can only merge SvDBRawObject instances,
 * so if you want to merge other object types, you have to
 * register custom merger function.
 *
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @param[in] typeName  object type name
 * @param[in] merger    merger function for objects of type @a typeName
 * @param[out] errorOut error info
 **/
extern void
SvDataBucketRegisterObjectMerger(SvDataBucket self,
                                 SvString typeName,
                                 SvDBObjectMergerFunc merger,
                                 SvErrorInfo *errorOut);

/**
 * Create an object in the data bucket from its XML representation.
 *
 * This method parses XML representation of a data bucket object
 * and adds it to the data bucket.
 *
 * @qb_allocator
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @param[in] XML       XML representation of data bucket objects
 * @param[out] errorOut error info
 * @return              created object or @c NULL in case of error
 **/
extern SvDBObject
SvDataBucketUpdate(SvDataBucket self,
                   const char *XML,
                   SvErrorInfo *errorOut);

/**
 * Create an object in the data bucket from its XML representation.
 *
 * This method is an extended version of SvDataBucketUpdate().
 * It returns information if created object, that have been
 * added to the data bucket, is new or different from previously
 * known instance of the same object.
 *
 * @qb_allocator
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @param[in] XML       XML representation of data bucket objects
 * @param[out] changed  @c true if contents of data bucket have changed
 * @param[out] errorOut error info
 * @return              created object or @c NULL in case of error
 **/
extern SvDBObject
SvDataBucketUpdateAndReport(SvDataBucket self,
                            const char *XML,
                            bool *changed,
                            SvErrorInfo *errorOut);

/**
 * Create an object from its XML representation.
 *
 * This method deserializes objects from their XML representation.
 * It returns an array of data bucket objects, where the last one
 * is the desired object and all other ones are referenced by it.
 *
 * @qb_allocator
 *
 * @param[in] XML       XML representation of data bucket objects
 * @param[out] errorOut error info
 * @return              created array of objects or @c NULL in case of parsing error
 **/
extern SvArray
SvDataBucketParse(const char *XML,
                  SvErrorInfo *errorOut);

/**
 * Create an object from its XML representation.
 *
 * This method does the same work as SvDataBucketParse(), but it reuses
 * auxiliary XML parser kept by the data bucket. This speeds up parsing,
 * but the parser object cannot be used in multiple threads simultaneously.
 *
 * @qb_allocator
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @param[in] XML       XML representation of data bucket objects
 * @param[in] length    length of @a XML in bytes, @c -1 if unknown
 * @param[out] errorOut error info
 * @return              created array of objects or @c NULL in case of parsing error
 **/
extern SvArray
SvDataBucketParseStateful(SvDataBucket self,
                          const char *XML,
                          ssize_t length,
                          SvErrorInfo *errorOut);

/**
 * Add previously created object to the data bucket.
 *
 * This method adds objects created by SvDataBucketParse() to the data
 * bucket. If the same object (identified by its type and unique ID)
 * is already present in the data bucket, it is updated and its handle
 * is returned instead of the new instance.
 *
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @param[in] objects   array of objects to be added
 * @param[out] errorOut error info
 * @return              object added to the data bucket, @c NULL in case of error
 **/
extern SvDBObject
SvDataBucketMerge(SvDataBucket self,
                  SvArray objects,
                  SvErrorInfo *errorOut);

/**
 * Add previously created object to the data bucket.
 *
 * This method is an extended version of SvDataBucketMerge().
 * It returns information if added object is new or different from
 * previously known instance of the same object.
 *
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @param[in] objects   array of objects to be added
 * @param[out] changed  @c true if contents of data bucket have changed
 * @param[out] errorOut error info
 * @return              object added to the data bucket, @c NULL in case of error
 **/
extern SvDBObject
SvDataBucketMergeAndReport(SvDataBucket self,
                           SvArray objects,
                           bool *changed,
                           SvErrorInfo *errorOut);

/**
 * Find object with given type and unique @a id in the data bucket.
 *
 * @qb_allocator
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 * @param[in] typeName  object type name
 * @param[in] id        object ID
 * @return              data bucket object or @c NULL if not found
 **/
extern SvDBObject
SvDataBucketLookup(SvDataBucket self,
                   SvString typeName,
                   SvValue id);

/**
 * Remove empty weak references from data bucket.
 *
 * @memberof SvDataBucket
 *
 * @param[in] self      data bucket handle
 **/
extern void
SvDataBucketCleanup(SvDataBucket self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

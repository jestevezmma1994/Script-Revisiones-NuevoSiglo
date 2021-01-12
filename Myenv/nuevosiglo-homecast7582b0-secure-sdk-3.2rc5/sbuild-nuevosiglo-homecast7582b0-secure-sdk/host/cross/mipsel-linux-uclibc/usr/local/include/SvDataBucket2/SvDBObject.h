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

#ifndef SV_DB_OBJECT_H_
#define SV_DB_OBJECT_H_

/**
 * @file SvDBObject.h Data bucket base object class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDBObject Data bucket object class
 * @ingroup SvDataBucket2
 * @{
 **/

/**
 * Data bucket object class.
 * @class SvDBObject
 * @extends SvObject
 **/
typedef struct SvDBObject_ {
    /// super class
    struct SvObject_ super_;
    /// type of the object: movie, channel etc.
    SvString type;
    /// object identifier, unique in scope of each type
    SvValue id;
} *SvDBObject;


/**
 * Unified data bucket object constructor.
 *
 * @param[in] type      object type
 * @param[in] id        object id
 * @param[in] attributes array containing even number of items; each pair
 *                      describes single attribute and consists of
 *                      name (SvString) and value (SvObject)
 * @return              created object or @c NULL in case of error
 **/
typedef SvDBObject
(*SvDBObjectConstructorFunc)(SvString type,
                             SvValue id,
                             SvArray attributes);

/**
 * Unified data bucket object merger function.
 *
 * @param [in] self     destination object
 * @param [in] source   source object
 * @return              @c true if destination object changed
 **/
typedef bool
(*SvDBObjectMergerFunc)(SvDBObject self,
                        SvDBObject source);


/**
 * Get runtime type identification object representing SvDBObject class.
 *
 * @return data bucket object class
 **/
extern SvType
SvDBObject_getType(void);

/**
 * Initialize data bucket object.
 *
 * This method initializes SvDBObject::type and SvDBObject::id fields
 * of data bucket object. It should be used in constructors of classes
 * derived from SvDBObject.
 *
 * @memberof SvDBObject
 *
 * @param[in] self      data bucket object handle
 * @param[in] type      object type
 * @param[in] id        object id
 * @return              @a self
 **/
extern SvDBObject
SvDBObjectInit(SvDBObject self,
               SvString type,
               SvValue id);

/**
 * Get data bucket object type.
 *
 * @memberof SvDBObject
 *
 * @param[in] self      data bucket object handle
 * @return              object type
 **/
extern SvString
SvDBObjectGetType(SvDBObject self);

/**
 * Get type of data bucket object as a C string.
 *
 * @memberof SvDBObject
 *
 * @param[in] self      data bucket object handle
 * @return              object type as a C string
 **/
extern const char *
SvDBObjectGetTypeAsCString(SvDBObject self);

/**
 * Get data bucket object ID.
 *
 * @memberof SvDBObject
 *
 * @param[in] self      data bucket object handle
 * @return              object ID
 **/
extern SvValue
SvDBObjectGetID(SvDBObject self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

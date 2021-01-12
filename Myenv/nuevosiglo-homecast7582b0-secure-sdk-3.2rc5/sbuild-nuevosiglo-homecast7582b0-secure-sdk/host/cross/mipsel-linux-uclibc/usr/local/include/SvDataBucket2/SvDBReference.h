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

#ifndef SV_DB_REFERENCE_H_
#define SV_DB_REFERENCE_H_

/**
 * @file SvDBReference.h Data bucket object reference class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvDataBucket2/SvDBObject.h>
#include <SvDataBucket2/SvDataBucket.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDBReference Data bucket object reference class
 * @ingroup SvDataBucket2
 * @{
 **/

/**
 * Data bucket object reference class.
 * @class SvDBReference
 * @extends SvDBObject
 **/
typedef struct SvDBReference_ *SvDBReference;


/**
 * Get runtime type identification object representing SvDBReference class.
 *
 * @return data bucket object reference class
 **/
extern SvType
SvDBReference_getType(void);

/**
 * Create data bucket object reference.
 *
 * @memberof SvDBReference
 *
 * @param[in] storage   data bucket handle (can be @c NULL)
 * @param[in] type      object type
 * @param[in] id        object id
 * @return              created reference, @c NULL in case of error
 **/
extern SvDBReference
SvDBReferenceCreate(SvDataBucket storage,
                    SvString type,
                    SvValue id);

/**
 * Set internal storage handle in data bucket object reference.
 *
 * @memberof SvDBReference
 *
 * @param[in] self      data bucket object reference
 * @param[in] storage   data bucket handle
 **/
extern void
SvDBReferenceAssignStorage(SvDBReference self,
                           SvDataBucket storage);

/**
 * Get referred object.
 *
 * @memberof SvDBReference
 *
 * @param[in] self      data bucket object reference
 * @return              referred object, @c NULL if it doesn't exist
 *                      in the data bucket
 **/
extern SvDBObject
SvDBReferenceGetReferredObject(SvDBReference self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

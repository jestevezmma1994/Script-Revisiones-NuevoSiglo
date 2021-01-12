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

#ifndef SV_DATA_BUCKET_ERROR_H_
#define SV_DATA_BUCKET_ERROR_H_

/**
 * @file SvDataBucketError.h DataBucket error domains
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDataBucketErrorDomain DataBucket error domain
 * @ingroup SvDataBucket2
 * @{
 **/

/**
 * DataBucket error domain.
 **/
#define SvDBErrorDomain "com.sentivision.SvDataBucket"

/**
 * DataBucket error codes.
 **/
typedef enum {
   /** reserved value (not a valid error code) */
   SvDBError_reserved = 0,
   /** trying to merge objects, that does not support merging */
   SvDBError_cannotMerge,
   /** marker for counting error codes */
   SvDBError_last = SvDBError_cannotMerge
} SvDBError;

/**
 * @}
 **/


/**
 * @defgroup SvDataBucketXMLErrorDomain DataBucket XML parser error domain
 * @ingroup SvDataBucket2
 * @{
 **/

/**
 * DataBucket XML parser error domain.
 **/
#define SvDBXMLErrorDomain "com.sentivision.SvDataBucket.SvDBParser"

/**
 * DataBucket XML parser error codes.
 **/
typedef enum {
   /** reserved value (not a valid error code) */
   SvDBXMLError_reserved = 0,
   /** XML data is not well-formed XML */
   SvDBXMLError_invalidXML,
   /** parser found an unknown XML element */
   SvDBXMLError_unexpectedElement,
   /** parser found valid XML element in wrong place */
   SvDBXMLError_misplacedElement,
   /** parser did not found a required XML element */
   SvDBXMLError_missingElement,
   /** a value of some XML element is invalid */
   SvDBXMLError_invalidValue,
   /** marker for counting error codes */
   SvDBXMLError_last = SvDBXMLError_invalidValue
} SvDBXMLError;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

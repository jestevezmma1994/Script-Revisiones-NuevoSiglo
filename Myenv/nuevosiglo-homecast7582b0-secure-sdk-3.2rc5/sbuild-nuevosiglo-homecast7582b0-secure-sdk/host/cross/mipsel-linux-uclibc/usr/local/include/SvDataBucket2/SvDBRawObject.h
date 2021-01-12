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

#ifndef SV_DB_RAW_OBJECT_H_
#define SV_DB_RAW_OBJECT_H_

/**
 * @file SvDBRawObject.h Data bucket raw object class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <QBOutputStream.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDBRawObject Data bucket raw object class
 * @ingroup SvDataBucket2
 * @{
 **/

/**
 * Data bucket raw object class.
 * @class SvDBRawObject
 * @extends SvDBObject
 **/
typedef struct SvDBRawObject_ *SvDBRawObject;

/**
 * Domains of object ids.
 * Each object can have differnt ids in every domain.
 **/

typedef enum {
    SvDBRawObjectDomain_Cubiware,      ///< Cubiware domain
    SvDBRawObjectDomain_DigitalSmiths  ///< DigitalSmiths domain
} SvDBRawObjectDomain;

/**
 * Get runtime type identification object representing SvDBRawObject class.
 *
 * @return data bucket raw object class
 **/
extern SvType
SvDBRawObject_getType(void);

/**
 * Create data bucket raw object.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] type      object type
 * @param[in] id        object id
 * @return              created raw object, @c NULL in case of error
 **/
extern SvDBRawObject
SvDBRawObjectCreate(SvString type,
                    SvValue id);

/**
 * Create data bucket raw object and fill it with attributes.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] type      object type
 * @param[in] id        object id
 * @param[in] attributes array containing even number of items; each pair
 *                      describes single attribute and consists of
 *                      name (SvString) and value (SvObject)
 * @return              created raw object, @c NULL in case of error
 **/
extern SvDBRawObject
SvDBRawObjectCreateWithAttributes(SvString type,
                                  SvValue id,
                                  SvArray attributes);

/**
 * Add data bucket raw object attribute.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 * @param[in] attrName  attribute name
 * @param[in] attrValue attribute value
 **/
extern void
SvDBRawObjectAddAttribute(SvDBRawObject self,
                          SvString attrName,
                          SvObject attrValue);

/**
 * Remove data bucket raw object attribute.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 * @param[in] attrName  attribute name
 **/
extern void
SvDBRawObjectRemoveAttribute(SvDBRawObject self,
                             const char *attrName);

/**
 * Get data bucket raw object attribute.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 * @param[in] attrName  attribute name
 * @return              attribute value, @c NULL if not found
 **/
extern SvObject
SvDBRawObjectGetAttrValue(SvDBRawObject self,
                          const char *attrName);

/**
 * Set data bucket raw object attribute.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 * @param[in] attrName  attribute name
 * @param[in] attrValue attribute value
 **/
extern void
SvDBRawObjectSetAttrValue(SvDBRawObject self,
                          const char *attrName,
                          SvObject attrValue);

/**
 * Shallow copy attributes from @a source to @a self.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      destination raw object handle
 * @param[in] source    source object handle
 * @return              @c true iff @a self was changed
 */
extern bool
SvDBRawObjectMerge(SvDBRawObject self,
                   SvDBRawObject source);

/**
 * Remove all attributes of a data bucket raw object.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 **/
extern void
SvDBRawObjectRemoveAllAttributes(SvDBRawObject self);

/**
 * Get iterator over names of all attributes of an object.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 * @return              iterator over attribute names
 **/
extern SvIterator
SvDBRawObjectGetAttributesIterator(SvDBRawObject self);

/**
 * Write a human readable representation of an object
 * to the stream.
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 * @param[in] stream    output stream to write to
 **/
extern void
SvDBRawObjectRepr(SvDBRawObject self,
                  QBOutputStream stream);

/**
 * Get value of boolean attribute.
 *
 * @memberof SvDBRawObject
 *
 * @param object        data bucket raw object handle
 * @param attrName      name of attribute
 * @return              value of attribute, @c false if there is no such
 *                      attribute or its type is incorrect
 **/
extern bool
SvDBRawObjectGetBoolAttr(SvDBRawObject object,
                         const char *attrName);

/**
 * Get value of string attribute.
 *
 * @param object        SvDBRawObject handle
 * @param attrName      Name of attribute
 * @return              Value of attribute. If no such attribite or type is incorrect function
 *                      returns false.
 **/
SvString
SvDBRawObjectGetStringAttr(SvDBRawObject object, const char *attrName);

/**
 * Set object id for a specific domain
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 * @param[in] domain   domain for which the id is needed.
 * @param[in] id        id value for a specific domain.
 **/
extern void
SvDBRawObjectSetDomainId(SvDBRawObject self,
                         SvDBRawObjectDomain domain,
                         SvValue id);

/**
 * Get object id for a specific domain
 *
 * @memberof SvDBRawObject
 *
 * @param[in] self      data bucket raw object handle
 * @param[in] domain   domain for which the id is needed.
 * @return              the id for specific domain, or general object id if domain specific id was not set.
 **/
extern SvValue
SvDBRawObjectGetDomainId(SvDBRawObject self,
                         SvDBRawObjectDomain domain);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

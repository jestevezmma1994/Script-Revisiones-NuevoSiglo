/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PROPERTIES_MAP_H_
#define QB_PROPERTIES_MAP_H_

/**
 * @file QBPropertiesMap.h
 * @brief Properties map class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvIterator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPropertiesMap Properties map class
 * @ingroup QBAppKit
 * @{
 **/

/**
 * Properties map class.
 * @class QBPropertiesMap
 * @extends SvObject
 **/
typedef struct QBPropertiesMap_ *QBPropertiesMap;


/**
 * Get runtime type identification object representing QBPropertiesMap class.
 *
 * @return properties map class
 **/
extern SvType
QBPropertiesMap_getType(void);

/**
 * Create empty properties map.
 *
 * @memberof QBPropertiesMap
 *
 * @param[out] errorOut error info
 * @return              created properties map, @c NULL in case of error
 **/
extern QBPropertiesMap
QBPropertiesMapCreate(SvErrorInfo *errorOut);

/**
 * Create properties map with contents read from JSON file.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] filePath  path to JSON file to read properties from
 * @param[out] errorOut error info
 * @return              created properties map, @c NULL in case of error
 **/
extern QBPropertiesMap
QBPropertiesMapCreateFromFile(const char *filePath,
                              SvErrorInfo *errorOut);

/**
 * Check if contents of a properties map can be written to JSON file.
 *
 * This method verifies the types of all properties stored in a map
 * to check if they can be written to JSON file. Only properties of
 * SvValue type holding string, integer, boolean or floating point value
 * are allowed in a map that can be written.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @return              @c true if map can be written to a file
 **/
extern bool
QBPropertiesMapIsSerializable(QBPropertiesMap self);

/**
 * Save properties map contents to JSON file.
 *
 * @note Only properties maps containing limited subset of SvValue type
 * properties can be written to JSON file, see QBPropertiesMapIsSerializable().
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] filePath  path to JSON file to write properties to
 * @param[out] errorOut error info
 **/
extern void
QBPropertiesMapWriteToFile(QBPropertiesMap self,
                           const char *filePath,
                           SvErrorInfo *errorOut);

/**
 * Get value of a property stored with specified @a key.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to get
 * @return              value of the property, @c NULL if not found
 **/
extern SvObject
QBPropertiesMapGetProperty(QBPropertiesMap self,
                           SvString key);

/**
 * Get value of a string property stored with specified @a key.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to get
 * @return              value of the property, @c NULL if property not found
 *                      or the property is not of string type
 **/
extern SvString
QBPropertiesMapGetStringProperty(QBPropertiesMap self,
                                 SvString key);

/**
 * Get value of a string property stored with specified @a key.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to get
 * @return              value of the property, @c NULL if property not found
 *                      or the property is not of string type
 **/
extern const char *
QBPropertiesMapGetStringPropertyAsCString(QBPropertiesMap self,
                                          SvString key);

/**
 * Get value of an integer property stored with specified @a key.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to get
 * @param[in] defaultValue default value
 * @return              value of the property, @a defaultValue if property not found
 *                      or the property is not of integer type
 **/
extern long long int
QBPropertiesMapGetIntProperty(QBPropertiesMap self,
                              SvString key,
                              long long int defaultValue);

/**
 * Get value of a boolean property stored with specified @a key.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to get
 * @param[in] defaultValue default value
 * @return              value of the property, @a defaultValue if property not found
 *                      or the property is not of boolean type
 **/
extern bool
QBPropertiesMapGetBooleanProperty(QBPropertiesMap self,
                                  SvString key,
                                  bool defaultValue);

/**
 * Set value of a property with specified @a key.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to set
 * @param[in] value     new value of the property
 * @param[out] errorOut error info
 **/
extern void
QBPropertiesMapSetProperty(QBPropertiesMap self,
                           SvString key,
                           SvObject value,
                           SvErrorInfo *errorOut);

/**
 * Set value of a string property with specified @a key.
 *
 * @note String property is stored as SvValue.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to set
 * @param[in] value     new value of the property
 * @param[out] errorOut error info
 **/
extern void
QBPropertiesMapSetStringProperty(QBPropertiesMap self,
                                 SvString key,
                                 SvString value,
                                 SvErrorInfo *errorOut);

/**
 * Set value of a string property with specified @a key.
 *
 * @note String property is stored as SvValue.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to set
 * @param[in] value     new value of the property
 * @param[out] errorOut error info
 **/
extern void
QBPropertiesMapSetCStringProperty(QBPropertiesMap self,
                                  SvString key,
                                  const char *value,
                                  SvErrorInfo *errorOut);

/**
 * Set value of an integer property with specified @a key.
 *
 * @note Integer property is stored as SvValue.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to set
 * @param[in] value     new value of the property
 * @param[out] errorOut error info
 **/
extern void
QBPropertiesMapSetIntProperty(QBPropertiesMap self,
                              SvString key,
                              long long int value,
                              SvErrorInfo *errorOut);

/**
 * Set value of a boolean property with specified @a key.
 *
 * @note Boolean property is stored as SvValue.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to set
 * @param[in] value     new value of the property
 * @param[out] errorOut error info
 **/
extern void
QBPropertiesMapSetBooleanProperty(QBPropertiesMap self,
                                  SvString key,
                                  bool value,
                                  SvErrorInfo *errorOut);

/**
 * Remove existing property.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @param[in] key       key of the property to remove
 * @param[out] errorOut error info
 **/
extern void
QBPropertiesMapRemoveProperty(QBPropertiesMap self,
                              SvString key,
                              SvErrorInfo *errorOut);

/**
 * Get iterator over keys of all properties in a map.
 *
 * @memberof QBPropertiesMap
 *
 * @param[in] self      properties map handle
 * @return              iterator over all keys
 **/
extern SvIterator
QBPropertiesMapGetKeysIterator(QBPropertiesMap self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCONFIGCORE_H_
#define QBCONFIGCORE_H_

/**
 * @file QBConfigCore.h QBConfigCore API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <stdbool.h>
#include <lightjson/lightjson_types.h>

/**
 * Initialize a QBConfigCore instance. Default configuration will be used.
 *
 * This method load configuration from const, user and default files
 */
extern void QBConfigCoreInit(void);

/**
 * Initialize a QBConfigCore instance with provided paths for configuration files.
 *
 * This method load configuration from provided const, user and default files
 *
 * @param[in] factoryConfigSpecFileName     a path to a factory configuration file
 * @param[in] userConfigSpecFileName        a path to an user configuration file
 * @param[in] defaultConfigSpecFileName     a path to a default configuration file
 * @param[in] tmpConfigCoreTemporaryFile    a path to a temporary configuration file (used internally by the QBConfigCore)
 */
extern void QBConfigCoreInitSpecific(const char *factoryConfigSpecFileName,
                                     const char *userConfigSpecFileName,
                                     const char *defaultConfigSpecFileName,
                                     const char *tmpConfigCoreTemporaryFile);

/**
 * Store Configuration in file
 */
extern int QBConfigCoreSave(void);

/**
 * Result of @c QBConfigCoreSet() call.
 */
typedef enum {
    QBConfigCoreSetResult_valueForKeyIsAlreadySet = -2,     ///< value for this key is already set, no change have been made then
    QBConfigCoreSetResult_fail = -1,                        ///< call failed
    QBConfigCoreSetResult_succes = 0                        ///< call succeeded
} QBConfigCoreSetResult;

/**
 * Set Key with value
 *
 * New key is set if its old value is different than new.
 * If key doesn't exist it is created.
 * Trying set key from const  configuration will abort application
 *
 * @param[in] key - keyName
 * @param[in] value - value of key, if NULL key is removed
 * @return error code
 */
extern QBConfigCoreSetResult QBConfigCoreSet(const char *key, const char *value);

/**
 * Get a string value mapped to a key.
 *
 * The value mapped the key is sought in a const config, then in an user config.
 * If the key is not found in both configurations then a default config is sought for the key.
 *
 * @param[in] key   configuration key
 * @return  a string value mapped to the key
 */
extern const char *QBConfigCoreGet(const char *key);

/**
 * Reset Key to default value
 *
 * Key is resetted to its default value.
 *
 * @param[in] key - keyName
 * @return error code
 */
extern int QBConfigCoreReset(const char *key);

/**
 * Get an integer value mapped to a key.
 *
 * The value mapped the key is sought in a const config, then in an user config.
 * If the key is not found in both configurations then a default config is sought for the key.
 *
 * @param[in] key       configuration key
 * @param[out] value    returned integer value that is mapped to the key
 * @return  0 if the key was found, -1 otherwise
 */
int QBConfigCoreGetInteger(const char *key, int *value);

/**
 * Get an object value mapped to a key.
 *
 * The value mapped the key is sought in a const config, then in an user config.
 * If the key is not found in both configurations then a default config is sought for the key.
 *
 * @param[in] key       configuration key
 * @param[out] p_value    returned object value that is mapped to the key
 * @return  0 if the key was found, -1 otherwise
 */
lightjson_type QBConfigCoreGetObject(const char *key, lightjson_value *p_value);

/**
 * Destroy configuration
 *
 * Removed configuration from memory
 */
extern void QBConfigCoreDestroy(void);


/**
 * Load configuration from file
 *
 * @param[in] filename - file with loaded configuration
 * @param[out] parsed - parsed configuration
 * @return - true if everything is ok
 */
extern bool QBConfigCoreLoadConfiguration(const char *filename, struct lightjson_object **parsed);

/**
 * Save configuration into specific file
 *
 * This method is serialize config to JSON, and store it into file
 *
 * @param[in] configuration - configuration keys
 * @param[in] filename - file with loaded configuration
 * @param[in] with_verification - choose whether HMAC verification/generation should be enforced (if supported by the platfomr).
 *            This parameter is really a compatibility feature for xml2json migration path - in such a case we do not want to
 *            generate HMAC for a migrated file even if HMAC enforcement is in effect - after all we're not sure whether those
 *            XML files are not malicious. So in most places with_verification is set to true, whereas in xml2json (QBConfig) is
 *            set to false.
 */
extern void QBConfigCoreSaveConfiguration(struct lightjson_object *configuration, const char *filename, bool with_verification);

/**
 * Set Key into specific configuration
 *
 * This allow to add a key to any of configuration area.
 * If key doesn't exist it will be added
 *
 * @param[in] object - configuration to insert a key
 * @param[in] key - key name (this method is used in recursion, so this attribute is storing part of not parsed key)
 * @param[in] value - value which will be inserted, if NULL key will be removed
 */
extern void QBConfigCoreSetKeyIntoConfiguration(struct lightjson_object *object, const char *key, const char *value);

/**
 * Find a key in specific configuration
 *
 * This allow to add a key to any of configuration area.
 * If key doesn't exist it will be added
 *
 * @param[in] config - configuration where to look in
 * @param[in] key - key name (this method is used in recursion, so this attribute is storing part of not parsed key)
 * @param[out] p_value - key value if exists or NULL if key does not have value or key doesn't store Value type
 * @return - p_value type
 */
extern lightjson_type QBConfigCoreFindKeyValueInConfiguration(struct lightjson_object *config, const char *key, lightjson_value *p_value);

/**
 * Find a object in specific configuration
 *
 * This allow to add a key to any of configuration area.
 * If key doesn't exist it will be added
 *
 * @param[in] p_config - configuration where to look in
 * @param[in] key - key name (this method is used in recursion, so this attribute is storing part of not parsed key)
 * @param[out] p_value - key object if exists or NULL if key does not exist
 * @return -  p_value type
 */
extern lightjson_type QBConfigCoreFindObjectInConfiguration(struct lightjson_object *p_config, const char *key, lightjson_value *p_value);


/**
 * Get keys from all configuration inserted into one config
 *
 * This method is returning keys with values from const, user and default configuration
 * If a specific key is in more that one configuration its value is taken in following order:
 * const -> user -> configuration
 *
 * @param[out] p_retTable - configuration including all found keys
 */
extern void QBConfigCoreGetAllKeys(struct lightjson_object *p_retTable);


#endif // QBCONFIGCORE_H_

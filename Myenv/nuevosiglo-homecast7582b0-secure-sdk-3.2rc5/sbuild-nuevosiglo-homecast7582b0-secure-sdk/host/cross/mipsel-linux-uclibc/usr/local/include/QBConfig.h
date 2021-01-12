/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCONFIG_H_
#define QBCONFIG_H_

#include <SvFoundation/SvCoreTypes.h>

typedef const struct QBConfigListener_t {
    void (*changed)(SvObject self_, const char *key, const char *value);
} *QBConfigListener;

extern void QBConfigAddListener(SvObject listener, const char *key);
extern void QBConfigRemoveListener(SvObject listener, const char *key);
extern SvInterface QBConfigListener_getInterface(void);

/**
 * Store Configuration in file
 */
extern int QBConfigSave(void);

/**
 * Result of @c QBConfigSet() call.
 */
typedef enum {
    QBConfigSetResult_valueForKeyIsAlreadySet = -2,     ///< value for this key is already set, no change have been made then
    QBConfigSetResult_fail = -1,                        ///< call failed
    QBConfigSetResult_succes = 0                        ///< call succeeded
} QBConfigSetResult;

/**
 * Set Key with value
 *
 * New key is set if its old value is different than new.
 * If key doesn't exist it is created.
 * Trying set key from const  configuration will abort application
 *
 * @param[in] key - keyName
 * @param[in] value - value of key, if NULL key is removed
 * @return    error code
 */
extern QBConfigSetResult QBConfigSet(const char *key, const char *value);

/**
 * Reset value of a configuration variable to default
 *
 * @param[in] key   variable name
 * @return          @c 0 on success, any other value otherwise
 */
extern int QBConfigReset(const char *key);

/**
 * Get Key value
 *
 * A Value for a key is seeking in const config, then in user.
 * If key is not found in that configuration default config is checking
 *
 * @param[in] key - keyName
 * @return - key value
 */
extern const char *QBConfigGet(const char *key);

/**
 * Get value of a configuration variable which is an integer type.
 *
 * @param[in] key    variable name
 * @param[out] value if function returns a success it has value of requested variable (otherwise it is unspecified)
 * @return           0 on success, other values signal error and value is not valid
 *
 * @remark Desired parameter can be stored in configuration in base 10 (default), base 16 (prefixed with "0x") or base 8 (prefixed with "0").
 **/
extern int QBConfigGetInteger(const char *key, int* value);

/**
 * Initialize a QBConfig instance.
 *
 * This method load configuration from const, user and default files
 */
extern void QBConfigInit(void);

/**
 * Destroy configuration
 *
 * Removed configuration from memory
 */
extern void QBConfigDestroy(void);

#endif  // QBCONFIG_H_

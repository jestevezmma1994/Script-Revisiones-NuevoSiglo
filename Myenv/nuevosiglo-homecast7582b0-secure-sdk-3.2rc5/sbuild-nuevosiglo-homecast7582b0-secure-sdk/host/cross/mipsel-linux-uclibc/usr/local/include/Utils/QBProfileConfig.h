/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PROFILE_CONFIG_H_
#define QB_PROFILE_CONFIG_H_

/**
 * @file QBProfileConfig.h
 * QBProfileConfig is singleton on top of the QBConfig, that implements user profile functionality.
 * QBProfileConfig observes QBConfig variable "UI.lastUserProfile" and changes profile config file accordingly to it,
 * so to change profile change "UI.lastUserProfile". If config file doesn't exist yet it will be created in @a PROFILES_DIR.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

typedef struct QBProfileConfig_ *QBProfileConfig;

/**
 * @defgroup QBProfileConfig Profile config util
 * @ingroup CubiTV_utils
 * @{
 **/

/**
 * Adds listener that listen for value change of key(s).
 *
 * New key is stored in QBConfig and profile config.
 *
 * @param[in] listener  listener that implements QBConfigListener interface
 * @param[in] key       key on which notification should be made, if NULL every key value change will notify
 */
extern void QBProfileConfigAddListener(SvObject listener, const char *key);

/**
 * Removes listener added with QBProfileConfigAddListener()

 *
 * @param[in] listener  listener that implements QBConfigListener interface
 * @param[in] key       key on which notification should be made, if NULL every key value change will notify
 */
extern void QBProfileConfigRemoveListener(SvObject listener, const char *key);

/**
 * Store Profile Configuration in file
 * @return error code
 */
extern int QBProfileConfigSave(void);

/**
 * Result of @c QBProfileConfigSet() call.
 */
typedef enum {
    QBProfileConfigSetResult_valueForKeyIsAlreadySet = -2,  ///< value for this key is already set, no change have been made then
    QBProfileConfigSetResult_fail = -1,                     ///< call failed
    QBProfileConfigSetResult_succes = 0                     ///< call succeeded
} QBProfileConfigSetResult;

/**
 * Set Key with value
 *
 * New key is stored in QBConfig and profile config.
 *
 * @param[in] key       key name
 * @param[in] value     value of key, if @c NULL key is removed
 * @return              error code
 */
extern QBProfileConfigSetResult QBProfileConfigSet(const char *key, const char *value);

/**
 * Get Key value
 *
 * @a Key is firstly seek in profile config, if not found there, another seek is made in QBConfig.
 *
 * @param[in] key       key name
 * @return              key value or @c NULL if key is not found
 */
extern const char *QBProfileConfigGet(const char *key);

/**
 * Get value of a profile configuration variable which is an integer type.
 *
 * @param[in] key       variable name
 * @param[out] value    if function returns a success it has value of requested variable (otherwise it is unspecified)
 * @return              0 on success, other values signal error and value is not valid
 **/
extern int QBProfileConfigGetInteger(const char *key, int* value);

/**
 * Initialize a QBProfileConfig instance.
 *
 * @param[out] errorOut     error output
 */
extern void QBProfileConfigInit(SvErrorInfo *errorOut);

/**
 * Destroys QBProfileConfig instance.
 */
extern void QBProfileConfigDestroy(void);

/**
 * @}
 **/

#endif /* QB_PROFILE_CONFIG_H_ */

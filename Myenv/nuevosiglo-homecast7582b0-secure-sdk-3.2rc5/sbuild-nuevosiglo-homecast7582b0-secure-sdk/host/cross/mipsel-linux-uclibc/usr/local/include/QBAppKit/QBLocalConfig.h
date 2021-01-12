/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2018 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBLOCALCONFIG_H
#define QBLOCALCONFIG_H

/**
 * @file QBLocalConfig.h
 * @brief Simple class for handling feature/component config json files.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBLocalConfig Local config handling
 * @ingroup QBAppKit
 * @{
 *
 * It is assumed that config files will be installed in ../CubiTV/Configurations.
 * Config handler can be created in read-only or read-write modes.
 *
 * Warning: Read-write mode affects only runtime values. Changes are visible by
 * other readers but aren't saved and are lost after STB's reboot.
 * It is advised to use read-write mode for testing purposes only.
 **/

/**
 * Method call results.
 **/
typedef enum {
    QBLocalConfigResult_error = -1,     ///< call failed
    QBLocalConfigResult_ok              ///< call succeeded
} QBLocalConfigResult;

/**
 * Access modes.
 **/
typedef enum {
    QBLocalConfigMode_readOnly,         ///< config is opened for read operations only
    QBLocalConfigMode_readWrite         ///< config is opened for read and write operations
} QBLocalConfigMode;

/**
 * Local config handling class.
 * @class QBLocalConfig
 * @extends SvObject
 **/
typedef struct QBLocalConfig_ *QBLocalConfig;

/**
 * Create QBLocalConfig object.
 *
 * @param[in]  fileName     config file name
 * @param[in]  mode         config data access mode
 * @param[out] errorOut     error info
 * @return local config handler instance, @c NULL in case of error
 */
QBLocalConfig
QBLocalConfigCreate(const char *fileName, QBLocalConfigMode mode, SvErrorInfo *errorOut);

/**
 * Get boolean config value.
 *
 * @param[in]  self         config handle
 * @param[in]  path         'path.to.value' - value's path in the data tree - string with '.' nodes separator
 * @param[out] valueOut     read value
 * @param[out] errorOut     error info
 * @return result of the operation
 */
QBLocalConfigResult
QBLocalConfigGetBool(QBLocalConfig self, const char *path, bool *valueOut, SvErrorInfo *errorOut);

/**
 * Set boolean config value.
 *
 * @param[in]  self         config handle
 * @param[in]  path         'path.to.value' - value's path in the data tree - string with '.' nodes separator
 * @param[out] newValue     new value
 * @param[out] errorOut     error info
 * @return result of the operation
 */
QBLocalConfigResult
QBLocalConfigSetBool(QBLocalConfig self, const char *path, bool newValue, SvErrorInfo *errorOut);

/**
 * Get integer config value.
 *
 * @param[in]  self         config handle
 * @param[in]  path         'path.to.value' - value's path in the data tree - string with '.' nodes separator
 * @param[out] valueOut     read value
 * @param[out] errorOut     error info
 * @return result of the operation
 */
QBLocalConfigResult
QBLocalConfigGetInteger(QBLocalConfig self, const char *path, long long int *valueOut, SvErrorInfo *errorOut);

/**
 * Set integer config value.
 *
 * @param[in]  self         config handle
 * @param[in]  path         'path.to.value' - value's path in the data tree - string with '.' nodes separator
 * @param[out] newValue     new value
 * @param[out] errorOut     error info
 * @return result of the operation
 */
QBLocalConfigResult
QBLocalConfigSetInteger(QBLocalConfig self, const char *path, long long int newValue, SvErrorInfo *errorOut);

/**
 * Get double config value.
 *
 * @param[in]  self         config handle
 * @param[in]  path         'path.to.value' - value's path in the data tree - string with '.' nodes separator
 * @param[out] valueOut     read value
 * @param[out] errorOut     error info
 * @return result of the operation
 */
QBLocalConfigResult
QBLocalConfigGetDouble(QBLocalConfig self, const char *path, double *valueOut, SvErrorInfo *errorOut);

/**
 * Set double config value.
 *
 * @param[in]  self         config handle
 * @param[in]  path         'path.to.value' - value's path in the data tree - string with '.' nodes separator
 * @param[out] newValue     new value
 * @param[out] errorOut     error info
 * @return result of the operation
 */
QBLocalConfigResult
QBLocalConfigSetDouble(QBLocalConfig self, const char *path, double newValue, SvErrorInfo *errorOut);

/**
 * Get string config value.
 *
 * @param[in]  self         config handle
 * @param[in]  path         'path.to.value' - value's path in the data tree - string with '.' nodes separator
 * @param[out] valueOut     read value
 * @param[out] errorOut     error info
 * @return result of the operation
 */
QBLocalConfigResult
QBLocalConfigGetString(QBLocalConfig self, const char *path, SvString *valueOut, SvErrorInfo *errorOut);

/**
 * Set string config value.
 *
 * @param[in]  self         config handle
 * @param[in]  path         'path.to.value' - value's path in the data tree - string with '.' nodes separator
 * @param[out] newValue     new value
 * @param[out] errorOut     error info
 * @return result of the operation
 */
QBLocalConfigResult
QBLocalConfigSetString(QBLocalConfig self, const char *path, SvString newValue, SvErrorInfo *errorOut);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // QBLOCALCONFIG_H

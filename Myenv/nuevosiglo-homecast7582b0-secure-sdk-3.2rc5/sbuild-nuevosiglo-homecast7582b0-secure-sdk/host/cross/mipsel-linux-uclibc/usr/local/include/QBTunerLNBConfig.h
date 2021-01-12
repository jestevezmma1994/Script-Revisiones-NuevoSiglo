/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011-2012 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_TUNER_LNB_CONFIG_H_
#define QB_TUNER_LNB_CONFIG_H_

/**
 * @file QBTunerLNBConfig.h Tuner LNB config class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBTunerTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTunerLNBConfig LNB configuration class
 * @ingroup QBTunerLibrary
 * @{
 **/

/**
 * LNB configuration class.
 * @class QBTunerLNBConfig
 * @extends SvObject
 **/
typedef struct QBTunerLNBConfig_ {
    /// base class
    struct SvObject_ super_;

    /// unique and never changing identifier of the configuration
    SvString ID;

    /// a set of LNB parameters
    struct QBTunerLNBParams LNB;
} *QBTunerLNBConfig;


/**
 * Get runtime type identification object
 * representing LNB configuration class.
 *
 * @return QBTunerLNBConfig type identification object
 **/
extern SvType
QBTunerLNBConfig_getType(void);

/**
 * Create new LNB configuration.
 *
 * @memberof QBTunerLNBConfig
 *
 * @param[in] satelliteID ID of the satellite
 * @param[in] LNBType LNB type
 * @param[out] errorOut error info
 * @return created LNB configuration object, @c NULL in case of error
 **/
extern QBTunerLNBConfig
QBTunerLNBConfigCreate(SvString satelliteID,
                       QBTunerLNBType LNBType,
                       SvErrorInfo *errorOut);

/**
 * Save new or modified LNB configuration.
 *
 * This method stores LNB configuration. If a configuration with
 * the same satellite name is already defined, this configuration
 * will replace it.
 *
 * @note QBStaticStorage must be initialized before calling this method.
 *
 * @param[in] self LNB configuration handle
 * @param[out] errorOut error info
 **/
extern void
QBTunerLNBConfigSave(QBTunerLNBConfig self,
                     SvErrorInfo *errorOut);


/**
 * Load all defined LNB configurations.
 *
 * This method loads all defined LNB configurations. It should
 * be called when satellite tuner is initialized to prepare for
 * using QBTunerLNBConfigGetByID().
 *
 * @param[out] errorOut error info
 **/
extern void
QBTunerLoadLNBConfig(SvErrorInfo *errorOut);

/**
 * Create list of all defined LNB configurations.
 *
 * @param[out] errorOut error info
 * @return created array of all known LNB configurations,
 *         must be released by the caller
 **/
extern SvArray
QBTunerCreateLNBConfigList(SvErrorInfo *errorOut);

/**
 * Remove existing LNB configuration.
 *
 * @memberof QBTunerLNBConfig
 *
 * @param[in] self LNB configuration handle
 * @param[out] errorOut error info
 **/
extern void
QBTunerLNBConfigRemove(QBTunerLNBConfig self,
                       SvErrorInfo *errorOut);

/**
 * @}
 **/


/**
 * @addtogroup QBTuner
 * @{
 **/

/**
 * Get LNB configuration by unique configuration identifier.
 *
 * @param[in] ID unique configuration identifier
 * @return LNB configuration with given ID, @c NULL if not found
 **/
extern QBTunerLNBConfig
QBTunerGetLNBConfigByID(SvString ID);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_TUNER_LNB_CONFIG_H_

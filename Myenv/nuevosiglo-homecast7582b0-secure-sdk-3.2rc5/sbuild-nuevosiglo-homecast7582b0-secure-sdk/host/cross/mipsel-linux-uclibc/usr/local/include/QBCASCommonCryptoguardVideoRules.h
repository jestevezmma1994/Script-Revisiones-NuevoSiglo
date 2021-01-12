/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCASCOMMONCRYPTOGUARDVIDEORULES_H_
#define QBCASCOMMONCRYPTOGUARDVIDEORULES_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCASCommonCryptoguardVideoRules.h QBCASCommonCryptoguardVideoRules class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCASCommonCryptoguardVideoRules QBCASCommonCryptoguardVideoRules
 * @ingroup QBCASCommonCryptoguard
 *
 * QBCASCommonCryptoguardVideoRules class
 */

/**
 * Rules for CGMSA
 */
typedef enum QBCASCommonCryptoguardVideoRulesCGMSA_e {
    QBCASCommonCryptoguardVideoRulesCGMSA_neutral, /**< neutral */
    QBCASCommonCryptoguardVideoRulesCGMSA_forcedDeactivation, /**< forced deactivation */
    QBCASCommonCryptoguardVideoRulesCGMSA_supportedUnitsOn, /**< supported units on */
    QBCASCommonCryptoguardVideoRulesCGMSA_mandatoryOnAllUnits, /**< mandatory on all units */
} QBCASCommonCryptoguardVideoRulesCGMSA;

/**
 * Rules for HDCP
 */
typedef enum QBCASCommonCryptoguardVideoRulesHDCP_e {
    QBCASCommonCryptoguardVideoRulesHDCP_neutral, /**< neutral */
    QBCASCommonCryptoguardVideoRulesHDCP_forcedDeactivation, /**< forced deactivation */
    QBCASCommonCryptoguardVideoRulesHDCP_supportedUnitsOn, /**< supported units on */
    QBCASCommonCryptoguardVideoRulesHDCP_mandatoryOnAllUnits, /**< mandatory on all units */
} QBCASCommonCryptoguardVideoRulesHDCP;

/**
 * Rules for Macrovision
 */
typedef enum QBCASCommonCryptoguardVideoRulesMacrovision_e {
    QBCASCommonCryptoguardVideoRulesMacrovision_neutral, /**< neutral */
    QBCASCommonCryptoguardVideoRulesMacrovision_forcedDeactivation, /**< forced deactivation */
    QBCASCommonCryptoguardVideoRulesMacrovision_supportedUnitsOn, /**< supported units on */
    QBCASCommonCryptoguardVideoRulesMacrovision_mandatoryOnAllUnits, /**< mandatory on all units */
} QBCASCommonCryptoguardVideoRulesMacrovision;

/**
 * Rules for Analog HD outputs
 */
typedef enum QBCASCommonCryptoguardVideoRulesAnalogHD_e {
    QBCASCommonCryptoguardVideoRulesAnalogHD_neutral, /**< neutral */
    QBCASCommonCryptoguardVideoRulesAnalogHD_maxResolution576i, /**< max resolution 576i */
} QBCASCommonCryptoguardVideoRulesAnalogHD;

/**
 * Rules for Digital HD outputs
 */
typedef enum QBCASCommonCryptoguardVideoRulesDigitalHD_e {
    QBCASCommonCryptoguardVideoRulesDigitalHD_neutral, /**< neutral */
    QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution1080p, /**< max resolution 1080p */
    QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution1080i, /**< max resolution 1080i */
    QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution720p, /**< max resolution 720p */
    QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution720i, /**< max resolution 720i */
    QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution576i, /**< max resolution 576i */
} QBCASCommonCryptoguardVideoRulesDigitalHD;


typedef struct QBCASCommonCryptoguardVideoRules_s {
    struct SvObject_ super_; /**< super type */

    QBCASCommonCryptoguardVideoRulesCGMSA cgmsa; /**< @ref QBCASCommonCryptoguardVideoRulesCGMSA */
    QBCASCommonCryptoguardVideoRulesHDCP hdcp; /**< @ref QBCASCommonCryptoguardVideoRulesHDCP */
    QBCASCommonCryptoguardVideoRulesMacrovision macrovision; /**< @ref QBCASCommonCryptoguardVideoRulesMacrovision */
    QBCASCommonCryptoguardVideoRulesAnalogHD analogHD; /**< @ref QBCASCommonCryptoguardVideoRulesAnalogHD */
    QBCASCommonCryptoguardVideoRulesDigitalHD digitalHD; /**< @ref QBCASCommonCryptoguardVideoRulesDigitalHD */
} *QBCASCommonCryptoguardVideoRules;

/**
 * Creates Video Rules message
 *
 * @param[out] errorOut error info
 * @return @ref QBCASCommonCryptoguardVideoRules
 */
QBCASCommonCryptoguardVideoRules QBCASCommonCryptoguardVideoRulesCreate(SvErrorInfo *errorOut);

/**
 * Get runtime type identification object representing @ref QBCASCommonCryptoguardVideoRules
 * @return @ref QBCASCommonCryptoguardVideoRules type
 **/
SvType QBCASCommonCryptoguardVideoRules_getType(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBCASCOMMONCRYPTOGUARDVIDEORULES_H_ */

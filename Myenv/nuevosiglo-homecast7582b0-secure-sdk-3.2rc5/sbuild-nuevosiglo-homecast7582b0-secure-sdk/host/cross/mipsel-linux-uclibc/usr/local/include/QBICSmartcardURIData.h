/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_INTEK_CONAX_SMARTCARD_URI_DATA_H
#define QB_INTEK_CONAX_SMARTCARD_URI_DATA_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvTime.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBICSmartcardURIData QBICSmartcardURIData
 * @{
 **/

/**
 * @file QBICSmartcardURIData.h Conax URI data handling interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/** Analog Protection System mode.
 * If STB is capable of encoding analogue outputs, appropriate
 * analog output protection should be applied. If STB is @b not
 * capable of encoding analogue output, STB should turn OFF
 * analog output on any value other then Off.
 */
typedef enum QBICSmartcardURIDataAPSCopyControl_e {
    QBICSmartcardURIDataAPSCopyControl_off,             /**< Analogue encoding off or/and analogue outputs on */
    QBICSmartcardURIDataAPSCopyControl_splitBurstOff,   /**< Analogue encoding on or analogue outputs off */
    QBICSmartcardURIDataAPSCopyControl_2LineSplitBurst, /**< Analogue encoding on or analogue outputs off */
    QBICSmartcardURIDataAPSCopyControl_4LineSplitBurst, /**< Analogue encoding on or analogue outputs off */
} QBICSmartcardURIDataAPSCopyControl;

/** Encryption Mode Indicator.
 * Describes content copying (i.e. PVR), moving and
 * displaying on digital outputs. If STB doesn't have HDCP on digital
 * outputs, digitial output should be off when HDCP is needed.
 */
typedef enum QBICSmartcardURIDataEMICopyControl_e {
    QBICSmartcardURIDataEMICopyControl_copyFreely, /**< HDCP can be off we can store and copy freely */
    QBICSmartcardURIDataEMICopyControl_copyOnce, /**< Store only once (i.e. on PVR) after that this flag is CopyNoMore, HDCP must be on */
    QBICSmartcardURIDataEMICopyControl_copyNoMore, /**< This flag is set on stored content with CopyOnce flag previously, HDCP must be on */
    QBICSmartcardURIDataEMICopyControl_copyNever /**< Long term storing is not allowed (i.e. PVR) (timeshift / trick play is allowed), HDCP must be on */
} QBICSmartcardURIDataEMICopyControl;

/** Image Constrained.
 * Describes analogue high definition outputs. If we
 * have high definition analogue output, and get lessThen520kPixels flag
 * we should display downsized content on that output. If we cannot
 * downscale content we must turn off that output.
 */
typedef enum QBICSmartcardURIDataImageConstraint_e {
    QBICSmartcardURIDataImageConstraint_off,  /**< We can display content on HD analogue outputs  */
    QBICSmartcardURIDataImageConstraint_lessThen520kPixels /**< Downscale content <520kPixels or disable output  */
} QBICSmartcardURIDataImageConstraint;

/** Redistribution Control mode.
 * If this flag is on, we must turn on HDCP on digital outputs
 * even if @ref QBICSmartcardURIDataEMICopyControl is set to CopyFreely
 */
typedef enum QBICSmartcardURIDataRedistributionControl_e {
    QBICSmartcardURIDataRedistributionControl_off, /**< HDCP on digitial outputs depends on other flags  */
    QBICSmartcardURIDataRedistributionControl_on /**< We must turn on HDCP on digitial outputs  */
} QBICSmartcardURIDataRedistributionControl;

/** Timeshfit Retention Limit.
 * Describes how long we can store content
 * when we do timeshift. We must care about that just and only when
 * @ref QBICSmartcardURIDataEMICopyControl is set to CopyNever
 * or CopyOnce. If @ref QBICSmartcardURIDataEMICopyControl is on CopyFreely
 * we assume NoLimit.
 */
typedef enum QBICSmartcardURIDataTimeshiftRetentionLimit_e {
    QBICSmartcardURIDataTimeshiftRetentionLimit_noLimit,    /**< Self explaining */
    QBICSmartcardURIDataTimeshiftRetentionLimit_1Week,      /**< Self explaining */
    QBICSmartcardURIDataTimeshiftRetentionLimit_2Days,      /**< Self explaining */
    QBICSmartcardURIDataTimeshiftRetentionLimit_1Day,       /**< Self explaining */
    QBICSmartcardURIDataTimeshiftRetentionLimit_12Hours,    /**< Self explaining */
    QBICSmartcardURIDataTimeshiftRetentionLimit_6Hours,     /**< Self explaining */
    QBICSmartcardURIDataTimeshiftRetentionLimit_3Hours,     /**< Self explaining */
    QBICSmartcardURIDataTimeshiftRetentionLimit_90Minutes   /**< Self explaining */
} QBICSmartcardURIDataTimeshiftRetentionLimit;

/** Longterm Retention Limit.
 * Describes how long we can store content
 * on PVR. We must care about that just and only when
 * @ref QBICSmartcardURIDataEMICopyControl is set to CopyOnce.
 * If @ref QBICSmartcardURIDataEMICopyControl is on CopyFreely
 * we assume NoLimit.
 */
typedef enum QBICSmartcardURIDataLongTermRetentionLimit_e {
    QBICSmartcardURIDataLongTermRetentionLimit_noLimit,     /**< Self explaining */
    QBICSmartcardURIDataLongTermRetentionLimit_360Days,     /**< Self explaining */
    QBICSmartcardURIDataLongTermRetentionLimit_90Days,      /**< Self explaining */
    QBICSmartcardURIDataLongTermRetentionLimit_30Days,      /**< Self explaining */
    QBICSmartcardURIDataLongTermRetentionLimit_14Days,      /**< Self explaining */
    QBICSmartcardURIDataLongTermRetentionLimit_7Days,       /**< Self explaining */
    QBICSmartcardURIDataLongTermRetentionLimit_2Days,       /**< Self explaining */
    QBICSmartcardURIDataLongTermRetentionLimit_1Day         /**< Self explaining */
} QBICSmartcardURIDataLongTermRetentionLimit;

/** Trick Play Control.
 * Describes what we can do with content
 * by Trick Playing
 */
typedef enum QBICSmartcardURIDataTrickPlayControl_e {
    QBICSmartcardURIDataTrickPlayControl_noRestriction,         /**< Self explaining */
    QBICSmartcardURIDataTrickPlayControl_noJumpingMaxFF2X,      /**< No skip/jump over frames, max fast forward 2X */
    QBICSmartcardURIDataTrickPlayControl_noJumpingMaxFF4X,      /**< No skip/jump over frames, max fast forward 4X */
    QBICSmartcardURIDataTrickPlayControl_noJumpingMaxFF8X,      /**< No skip/jump over frames, max fast forward 8X */
    QBICSmartcardURIDataTrickPlayControl_noJumpingNoFF,         /**< No skip/jump over frames or fast forward */
    QBICSmartcardURIDataTrickPlayControl_noPauseNoJumpingNoFF,  /**< No skip/jump over frames, fast forward and pause */
    QBICSmartcardURIDataTrickPlayControl_reserved,              /**< No valid value, this param should be ommited  */
} QBICSmartcardURIDataTrickPlayControl;

/** Maturity Rating.
 * Describes maturity ratings for content
 */
typedef enum QBICSmartcardURIDataMaturityRating_e {
    QBICSmartcardURIDataMaturityRating_G = 0x1,              /**< Over 0 year */
    QBICSmartcardURIDataMaturityRating_PG = 0x2,             /**< Over 10 year */
    QBICSmartcardURIDataMaturityRating_A = 0x4,              /**< Over 18 year */
    QBICSmartcardURIDataMaturityRating_X = 0x8,              /**< Over 18 year, erotic content */
    QBICSmartcardURIDataMaturityRating_XXX = 0x9,            /**< Over 18 year, erotic content, PIN required */
    QBICSmartcardURIDataMaturityRating_reserved = -1,        /**< No valid value, this param should be ommited  */
} QBICSmartcardURIDataMaturityRating;

/** Disable Analogue Outputs.
 *  Describes if we can enable analogue outputs.
 */
typedef enum QBICSmartcardURIDataDisableAnalogueOutputs_e {
    QBICSmartcardURIDataDisableAnalogueOutputs_off, /**< Analogue outputs can be enabled */
    QBICSmartcardURIDataDisableAnalogueOutputs_on,  /**< Analogue outputs must be disabled */
} QBICSmartcardURIDataDisableAnalogueOutputs;

/** This structure is optional in Conax 6.0.
 * We must parse it but we don't support it
 */
typedef struct QBICSmartcardURIDataExportControl_s {
    bool isValid;

    bool ciplusExportAllowed;
    bool pbdaExportAllowed;
    bool dtcpIpExportAllowed;
    bool hnDrmExportAllowed;
} QBICSmartcardURIDataExportControl;

/** This structure is optional in Conax 6.0.
 * We must parse it but we don't support it
 */
typedef struct QBICSmartcardURIDataExportConditions_s {
    bool isValid;
} QBICSmartcardURIDataExportConditions;

/** URI structure containing enums described
 * earlier
 */
typedef struct QBICSmartcardURIDataUsageRulesInformation_s {
    QBICSmartcardURIDataAPSCopyControl apsCopyControl;
    QBICSmartcardURIDataEMICopyControl emiCopyControl;
    QBICSmartcardURIDataImageConstraint imageConstraint;
    QBICSmartcardURIDataRedistributionControl redistributionControl;
    QBICSmartcardURIDataTimeshiftRetentionLimit timeshiftRetentionLimit;
    QBICSmartcardURIDataLongTermRetentionLimit longTermRetentionLimit;
    QBICSmartcardURIDataTrickPlayControl trickPlayControl;
    QBICSmartcardURIDataMaturityRating maturityRating;
    QBICSmartcardURIDataDisableAnalogueOutputs disableAnalogueOutputs;
} QBICSmartcardURIDataUsageRulesInformation;

/** Conax smart card URI class
 */
typedef struct QBICSmartcardURIData_t {
    struct SvObject_ super_;
    QBICSmartcardURIDataExportControl exportControl;
    QBICSmartcardURIDataExportConditions exportConditions;
    QBICSmartcardURIDataUsageRulesInformation usageRulesInformation;
    double offset;
} * QBICSmartcardURIData;

/** Creates default URI object, described in Conax 6.0 specification.
 */
extern QBICSmartcardURIData QBICSmartcardURIDataCreateDefault(void);

/** This function set the least restriction on URI object @a self. We using
 * it for FTA channels.
 * @param self reference to object to be filled
 */
extern void QBICSmartcardURIDataSetNoRestriction(QBICSmartcardURIData self);

/** Get runtime type identification object representing QBICSmartcardURIData class
 * @return object representing QBICSmartcardURIData class
 */
extern SvType QBICSmartcardURIData_getType(void);

/** Creates URI object,by parsing @ref QBConaxNotificationUriData raw data into
 * @ref QBICSmartcardURIData object.
 * @param notificationUriData @ref QBConaxNotificationUriData
 */
extern QBICSmartcardURIData QBICSmartcardURIDataCreate(const uint8_t* data, size_t size);

/** Translates longterm retention limit to SvTime.
 * @param longTermRetentionLimit longterm retention limit.
 * @return SvTime with retention limit time.
 */
extern SvTime QBICSmartcardURIDataLongTermRetentionLimitToTime(const QBICSmartcardURIDataLongTermRetentionLimit longTermRetentionLimit);

/** Translates timeshift retention limit to SvTime.
 * @param timeshiftRetentionLimit timeshift retention limit
 * @return SvTime with retention limit time.
 */
extern SvTime QBICSmartcardURIDataTimeshiftRetentionLimitToTime(const QBICSmartcardURIDataTimeshiftRetentionLimit timeshiftRetentionLimit);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_INTEK_CONAX_SMARTCARD_URI_DATA_H

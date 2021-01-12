/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 - 2015 Cubiware Sp. z o.o. All rights reserved.
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
****************************************************************************/

#ifndef QB_CONTENT_PROTECTION_H
#define QB_CONTENT_PROTECTION_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum QBContentProtection_e
{
  /// protection level is unknown
  QBContentProtection_unknown = 0,
  /// disable content protection
  QBContentProtection_disable,
  /// content protection is optional
  QBContentProtection_optional,
  /// content protection is required for HD output, optional for SD
  QBContentProtection_required_hd,
  /// content protection is required
  QBContentProtection_required,
  /// content can't be sent to the output at all
  QBContentProtection_no_output,
};
typedef enum QBContentProtection_e  QBContentProtection;

enum QBContentProtectionPriority_e
{
  //Lowest priority
  QBContentProtectionPriority_unknown = 0,
  QBContentProtectionPriority_content,
  QBContentProtectionPriority_application,
  QBContentProtectionPriority_platform
  //Highest priority
};
typedef enum QBContentProtectionPriority_e  QBContentProtectionPriority;

enum QBContentProtectionCGMSType_e
{
    QBContentProtectionCGMSType_unknown = 0,
    QBContentProtectionCGMSType_A,
    QBContentProtectionCGMSType_B
};
typedef enum QBContentProtectionCGMSType_e  QBContentProtectionCGMSType;

enum QBContentProtectionCGMSMode_e
{
  QBContentProtectionCGMSMode_copyFreely = 0,
  QBContentProtectionCGMSMode_copyNoMore, // HD only
  QBContentProtectionCGMSMode_copyOnce,
  QBContentProtectionCGMSMode_copyNever
};
typedef enum QBContentProtectionCGMSMode_e  QBContentProtectionCGMSMode;

enum QBContentProtectionASBMode_e
{
  QBContentProtectionASBMode_notAnalogMedium = 0,
  QBContentProtectionASBMode_analogMedium
};
typedef enum QBContentProtectionASBMode_e  QBContentProtectionASBMode;

enum QBContentProtectionTVAspectRatio_e
{
    QBContentProtectionTVAspectRatio_unknown = 0,
    QBContentProtectionTVAspectRatio_4x3,
    QBContentProtectionTVAspectRatio_16x9
};
typedef enum QBContentProtectionTVAspectRatio_e QBContentProtectionTVAspectRatio;

/*
 * Macrovsion settings modes
 */
enum QBContentProtectionMacrovisionMode_e
{
    /** Informations about macrovision are not provided */
    QBContentProtectionMacrovisionMode_unknown = 0,
    /** Macrovision should be disabled */
    QBContentProtectionMacrovisionMode_disabled,
    /** CPC value is provided */
    QBContentProtectionMacrovisionMode_customCpc,
    /** Only automatic gain control */
    QBContentProtectionMacrovisionMode_agcOnly,
    /** automatic gain control with 2 lines color stripping */
    QBContentProtectionMacrovisionMode_agc2Lines,
    /** automatic gain control with 4 lines color stripping */
    QBContentProtectionMacrovisionMode_agc4Lines,
};
typedef enum QBContentProtectionMacrovisionMode_e QBContentProtectionMacrovisionMode;

/*
 * Macrovision settings
 */
struct QBContentProtectionMacrovisionSettings_s
{
    /* Macrovision settings mode **/
    QBContentProtectionMacrovisionMode mode;
    /** Value used in customCpc mode*/
    uint8_t cpc;
};

typedef struct QBContentProtectionMacrovisionSettings_s QBContentProtectionMacrovisionSettings;


struct sv_content_protection
{
  struct {
    /// protection of compressed (AC3 or DTS) digital audio on S/PDIF output
    struct {
      QBContentProtection scms_level; /// SCMS content protection level
      uint8_t scms_value;             /// SCMS bits, set to @c 0 if not applicable
    } spdif_compressed_audio;

    /// protection of uncompressed (PCM) digital audio on S/PDIF output
    struct {
      QBContentProtection scms_level; /// SCMS content protection level
      uint8_t scms_value;             /// SCMS bits, set to @c 0 if not applicable
    } spdif_uncompressed_audio;

    /// protection of compressed (any codec) digital audio on HDMI output
    struct {
      QBContentProtection hdcp_level; /// HDCP content protection level
    } hdmi_compressed_audio;

    /// protection of uncompressed (PCM) digital audio on HDMI output
    struct {
      QBContentProtection hdcp_level; /// HDCP content protection level
    } hdmi_uncompressed_audio;

    /// protection of video on SD analog outputs
    struct {
      QBContentProtection cgms_level;             /// Analog content protection level
      QBContentProtectionCGMSMode cgms_mode;            /// CGMS-A bits, set to @c 0 if not applicable
      QBContentProtectionASBMode asb_mode;              /// ASB bits
      QBContentProtectionTVAspectRatio aspect_ratio;
      QBContentProtectionMacrovisionSettings macrovision_settings;
      bool hd_component_allowed;      /// @c true if HD video is allowed on component (YPbPr or RGB) output
    } analog_video;

    /// protection of uncompressed video on HDMI output
    struct {
      QBContentProtection hdcp_level; /// HDCP content protection level
    } hdmi_video;
  } protection;

  QBContentProtectionPriority priority; /// Priority of protection info
};


extern void QBContentProtectionInit(struct sv_content_protection* contentProtectionInfo);

extern const char* QBContentProtectionToString(QBContentProtection cp);

extern unsigned int QBContentProtectionCGMSGetDataWithCRC(unsigned int ulData, bool isBigEndian);

/**
 * Returns APS bits calculated from Macrovision settings
 *
 * @param[in] macrovisionSettings supported modes: disabled, agcOnly, agc2Lines, agc4Lines. With other modes behavior is undefined.
 * @return calculated bits value
 */
extern uint8_t QBContentProtectionCalculateAPSBits(const QBContentProtectionMacrovisionSettings *macrovisionSettings);
/**
 * Returns VBI bits calculated from Analog Protections settings
 *
 * @param[in] cgmsMode CGMS-A protection bits
 * @param[in] macrovisionSettings @see QBContentProtectionCalculateAPSBits
 * @param[in] asbMode ASB protection bits
 * @return calculated bits value
 */
extern uint8_t QBContentProtectionCalculateVBIValue(QBContentProtectionCGMSMode cgmsMode, const QBContentProtectionMacrovisionSettings *macrovisionSettings, QBContentProtectionASBMode asbMode);

/**
 * Returns WSS bits calculated from CGMS protection mode and TV aspect ratio
 *
 * @param[in] cgmsMode CGMS-A protection bits
 * @param[in] aspectRatio TV aspect ratio
 * @return calculated bits value
 */
extern unsigned int QBContentProtectionCalculateVBIValueWSS(QBContentProtectionCGMSMode cgmsMode, QBContentProtectionTVAspectRatio aspectRatio);

/**
 * Returns Macrovision CPS table based on macrovision settings
 *
 * @param[in] macrovision_settings: handle to macrovision settings
 * @return handle to 17 bytes length array with cps values
 */
extern const uint8_t* QBContentProtectionGetMacrovisionCpsTable(const QBContentProtectionMacrovisionSettings *macrovision_settings);

/** Mask of APS data bits @see QBContentProtectionCalculateMacrovisionMode */
#define QB_CONTENT_PROTECTION_APS_DATA_BITS_MASK 0x03

/**
 * Calculate macrovison mode from APS bits
 *
 * @param[in] apsBits 2 bit information (if bits other that two least significant are set then behavior is undefined)
 * @return Macrovision mode
 */
extern QBContentProtectionMacrovisionMode QBContentProtectionCalculateMacrovisionMode(uint8_t apsBits);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_CONTENT_PROTECTION_H

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

#ifndef QB_PLATFORM_OUTPUT_H_
#define QB_PLATFORM_OUTPUT_H_

/**
 * @file QBPlatformOutput.h Video outputs control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBPlatformVideoOutput Video outputs control
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Maximal number of master video outputs.
 **/
#define QB_PLATFORM_MAX_MASTER_OUTPUTS_COUNT 4

/**
 * Get number of available video outputs.
 *
 * @return                  number of video outputs, @c -1 in case of error
 **/
extern int
QBPlatformGetOutputsCount(void);

/**
 * Find video output with requested capabilities.
 *
 * This function searches all video outputs for one of specified type
 * and/or given name and/or capabilities. You can also specify whether
 * you're interested only in master outputs and only unused ones.
 *
 * @param[in] type          requested video output type, pass QBOutputType_unknown
 *                          to search by other criteria
 * @param[in] name          requested video output name, pass @c NULL to search
 *                          only by other criteria
 * @param[in] capabilities  binary set of requested capabilities, @c 0 will match
 *                          any output
 * @param[in] masterOnly    search only for master outputs
 * @param[in] unusedOnly    search only in currently unused outputs
 * @return                  ID of the matching output, @c -1 if not found
 **/
extern int
QBPlatformFindOutput(QBOutputType type,
                     const char *name,
                     unsigned int capabilities,
                     bool masterOnly,
                     bool unusedOnly);

/**
 * Get current configuration of the video output.
 *
 * This function returns current configuration of one of video outputs.
 *
 * Video outputs are identified by consecutive numbers, starting with @c 0.
 * To probe for all available outputs, application code shall call this
 * function while incrementing @a outputID until error occurs.
 *
 * @param[in] outputID  video output ID, starting from @c 0
 * @param[out] cfg      current configuration of video output identified by
 *                      @a outputID
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformGetOutputConfig(unsigned int outputID,
                          QBVideoOutputConfig *cfg);

/**
 * Get name of the video output.
 *
 * @param[in] outputID  video output ID, starting from @c 0
 * @return              output name on success, @c NULL in case of error
 **/
extern const char *
QBPlatformGetOutputName(unsigned int outputID);

/**
 * Set HDCP on specific output.
 *
 * Levels QBContentProtection_disable or
 * QBContentProtection_required with highest
 * priority are set, other levels are ignored.
 *
 * @param[in] outputID  video output ID
 * @param[in] level see @a QBContentProtection
 * @param[in] priority source of output control (i.e. content, application, platform)
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetHDCP(unsigned int outputID,
                  QBContentProtection level,
                  QBPlatformOutputPriority priority);

/**
 * Set CGMS on specific output.
 *
 * Levels QBContentProtection_disable or
 * QBContentProtection_required with highest
 * priority are set, other levels are ignored.
 *
 * @param[in] outputID  video output ID
 * @param[in] level see @a QBContentProtection
 * @param[in] mode see @a QBContentProtectionCGMSMode
 * @param[in] priority source of output control (i.e. content, application, platform)
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetCGMS(unsigned int outputID,
                  QBContentProtection level,
                  QBContentProtectionCGMSMode mode,
                  QBPlatformOutputPriority priority);

/**
 * Set Microvision Analog Protection System on specific output.
 *
 * If one of priorities has @a useAGCAndCS on true,
 * APS will be set, with highest priority mode.
 *
 * @param[in] outputID  video output ID
 * @param[in] macrovisionSettings see @a QBContentProtectionMacrovisionSettings
 * @param[in] priority source of output control (i.e. content, application, platform)
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetMacrovision(unsigned int outputID,
                         const QBContentProtectionMacrovisionSettings *macrovisionSettings,
                         QBPlatformOutputPriority priority);

/**
 * Update master output content protection configuration using current level requests.
 *
 * It detects capabilities of output, and call QBPlatformSetAPS(), QBPlatformSetCGMS(),
 * and QBPlatformSetHDCP() accordingly.
 *
 * @param[in] outputID  video output ID
 * @param[in] config    content protection config
 * @param[in] priority  source of output control (i.e. content, application, platform)
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformUpdateContentProtection(unsigned int outputID,
                                  const struct sv_content_protection *const config,
                                  QBPlatformOutputPriority priority);

/**
 * Enables output with provided mode and aspect ratio
 *
 * @param[in] outputID          video output ID (see QBPlatformGetOutputConfig())
 * @param[in] mode              starting standard mode
 * @param[in] aspectRatio      starting aspect ratio
 * @return                      @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformOutputEnable(unsigned int outputID,
                       QBOutputStandard mode,
                       QBAspectRatio aspectRatio);\

/**
 * Disables output
 *
 * QBPlatformOutputDisable
 * @param[in] outputID          video output ID (see QBPlatformGetOutputConfig())
 * @return                      @c 0 on success, @c -1 in case of error
 */
extern int
QBPlatformOutputDisable(unsigned int outputID);

/**
 * Update value of the CEC status in video output configuration.
 * Needs to be called with QBPlatformWRLock held
 *
 * @param[in] outputID  video output ID
 * @param[in] status    CEC status
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetOutputCECStatus(unsigned int outputID,
                             const QBCECStatus status);

/**
 * Get output CEC status.
 *
 * @param[in] outputID  video output ID
 * @param[out] status   CEC status
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformGetOutputCECStatus(unsigned int outputID,
                             QBCECStatus status);

/**
 * Enable or disable HDMI-CEC.
 *
 * @param[in] outputID  video output ID
 * @param[out] enable   @c true if CEC should be enabled, @c false otherwise
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int QBPlatformOutputCECEnable(unsigned int outputID,
                                     bool enable);

/**
 * Power off/on specific video output
 *
 * If one of priorities has @a powerOff on true,
 * output will be powered off.
 *
 * @param[in] outputID  video output ID
 * @param[in] powerOff @c true if output should be powered off, @c false if we want to power on output
 * @param[in] priority source of output control (i.e. content, application, platform)
 * @return              @c 0 on success, @c -1 in case of error
 */
extern int
QBPlatformOutputSetPowerOff(unsigned int outputID,
                            bool powerOff,
                            QBPlatformOutputPriority priority);

/**
 * Mute off/on video on output
 *
 * If one of priorities has @a mute on true,
 * output will be mutted.
 *
 * @param[in] outputID  video output ID
 * @param[in] mute @c true if video on output should be disabled, @c false if video on output should be enabled
 * @param[in] priority source of output control (i.e. content, application, platform)
 * @return              @c 0 on success, @c -1 in case of error
 */
extern int
QBPlatformOutputMute(unsigned int outputID,
                     bool mute,
                     QBPlatformOutputPriority priority);

/**
 * Set standard on output
 *
 * Standard with smallest resolution off all priorities
 * will be set
 *
 * @param[in] outputID  video output ID
 * @param[in] standard see @ref QBOutputStandard
 * @param[in] priority source of output control (i.e. content, application, platform)
 * @return              @c 0 on success, @c -1 in case of error
 */
extern int
QBPlatformOutputSetStandard(unsigned int outputID,
                            QBOutputStandard standard,
                            QBPlatformOutputPriority priority);

/**
 * Set aspect ratio on output
 *
 * AspecRatio with highest priority which is
 * not @c QBAspectRatio_unknown will be set.
 *
 * @param[in] outputID  video output ID
 * @param[in] aspectRatio see @ref QBAspectRatio
 * @param[in] priority source of output control (i.e. content, application, platform)
 * @return              @c 0 on success, @c -1 in case of error
 */
extern int
QBPlatformOutputSetAspectRatio(unsigned int outputID,
                               QBAspectRatio aspectRatio,
                               QBPlatformOutputPriority priority);

/**
 * Set PAL system (used only for PAL output standard)
 *
 * @param[in] outputID  video output ID
 * @param[in] palSystem    PAL system
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetPALSystem(unsigned int outputID,
                       QBPALSystem palSystem);

/**
 * Check if given @a mode is supported by video output.
 *
 * @note This function only verifies the capabilities of the output hardware
 * and drivers, it won't check if mode is supported by connected receiver:
 * see QBPlatformHDMIOutputGetEDID() for such functionality.
 *
 * @param[in] outputID  video output ID, starting from @c 0
 * @param[in] mode      video mode to check
 * @param[out] isSupported @c true if @a mode is supported, @c false otherwise
 *                      (valid only when function succeeds)
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformOutputIsModeSupported(unsigned int outputID,
                                QBOutputStandard mode,
                                bool *isSupported);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif

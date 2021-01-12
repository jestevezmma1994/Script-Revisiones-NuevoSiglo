/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PLATFORM_UTIL_H_
#define QB_PLATFORM_UTIL_H_

/**
 * @file QBPlatformUtil.h Platform HAL utility functions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformUtils Platform HAL utilities
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Get name of the TV system type.
 *
 * @param[in] system        TV system type
 * @return                  printable name of the TV @a system
 **/
extern const char *
QBPlatformGetTVSystemName(QBTVSystem system);

/**
 * Parse TV system type from name.
 *
 * @param[in] name          TV system name
 * @return                  Tv system type, QBTVSystem::QBTVSystem_none
 *                          if not recognized
 **/
extern QBTVSystem
QBPlatformGetTVSystemByName(const char *name);

/**
 * Calculate preferred TV system from video framerate and number of lines
 * in a picture.
 *
 * @param[in] linesCnt      number of lines in a frame
 * @param[in] frameRate     number of frames per second * 1000
 * @return                  selected TV system, QBTVSystem::QBTVSystem_none
 *                          if it can't be detected from given values
 **/
extern QBTVSystem
QBPlatformGetTVSystemByVideoParams(unsigned int linesCnt,
                                   unsigned int frameRate);

/**
 * Get name of the video output standard.
 *
 * @param[in] mode          video output standard
 * @return                  printable name of the @a mode
 **/
extern const char *
QBPlatformGetModeName(QBOutputStandard mode);

/**
 * Get video output standard by its full set of parameters.
 *
 * @param[in] height        number of visible picture rows
 * @param[in] refreshRate   number of vertical syncs per second, rounded down
 * @param[in] interlaced    @c true to find only interlaced video modes
 * @return                  output standard, QBOutputStandard::QBOutputStandard_none
 *                          if not recognized
 **/
extern QBOutputStandard
QBPlatformFindMode(unsigned int height,
                   unsigned int refreshRate,
                   bool interlaced);

/**
 * Get video output standard by its name and/or TV system.
 *
 * @param[in] name          canonical name of the output standard
 * @param[in] system        TV system in case @a name is ambiguous
 * @return                  output standard, QBOutputStandard::QBOutputStandard_none
 *                          if not recognized
 **/
extern QBOutputStandard
QBPlatformFindModeByName(const char *name,
                         QBTVSystem system);

/**
 * Get output dimensions of the video output standard.
 *
 * @param[in] mode          video output standard
 * @param[out] width        video output width
 * @param[out] height       video output height
 * @return                  @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformGetModeDimensions(QBOutputStandard mode,
                            unsigned int *width,
                            unsigned int *height);

/**
 * Get refresh rate of the video output standard.
 *
 * @param[in] mode          video output standard
 * @return                  refresh rate in millihertz, @c -1 in case of error
 **/
extern int
QBPlatformGetModeRefreshRate(QBOutputStandard mode);

/**
 * Get name of the video output aspect ratio.
 *
 * @param[in] aspectRatio   video output aspect ratio
 * @return                  printable name of the @a aspectRatio
 **/
extern const char *
QBPlatformGetAspectName(QBAspectRatio aspectRatio);

/**
 * Get name of the video output type.
 *
 * @param[in] type          video output type
 * @return                  printable name of the output @a type
 **/
extern const char *
QBPlatformGetOutputTypeName(QBOutputType type);

/**
 * Get name of the viewport mode.
 *
 * @param[in] mode          viewport mode
 * @return                  printable name of the viewport @a mode
 **/
extern const char *
QBPlatformGetViewportModeName(QBViewportMode mode);

/**
 * Get name of the video content display mode.
 *
 * @param[in] mode          video content display mode
 * @return                  printable name of the display @a mode
 **/
extern const char *
QBPlatformGetContentDisplayModeName(QBContentDisplayMode mode);

/**
 * Get the video content display mode by name.
 *
 * @param[in] name          name of the display mode
 * @return                  video content display mode
 **/
extern QBContentDisplayMode
QBPlatformFindContentDisplayModeByName(const char* name);

/**
 * Get name of the audio output type.
 *
 * @param[in] type          audio output type
 * @return                  printable name of the output @a type
 **/
extern const char *
QBPlatformGetAudioOutputTypeName(QBAudioOutputType type);

/**
 * Retrieve video outputs configuration from system configuration,
 * environment variables and command line parameters (in that order).
 *
 * @param[in] cfg           an array for video outputs configuration
 * @param[in] count         number of elements in @a cfg array
 * @param[in] system        local TV system
 * @param[in,out] argc      number of command line parameters in @a argv
 * @param[in,out] argv      command line parameters, updated on output
 *                          (with video output parameters removed)
 * @return                  number of video outputs we've got configuration
 *                          for (can be greater than @a count),
 *                          @c -1 in case of error
 **/
extern int
QBPlatformParseVideoOutputsConfiguration(QBVideoOutputConfig *cfg,
                                         size_t count,
                                         QBTVSystem system,
                                         int *argc, char ***argv);

/**
 * Get default video outputs configuration for given TV system.
 *
 * @param[in] cfg           an array for video outputs configuration
 * @param[in] count         number of elements in @a cfg array
 * @param[in] system        local TV system
 * @return                  number of video outputs we've got configuration
 *                          for (can be greater than @a count),
 *                          @c -1 in case of error
 **/
extern int
QBPlatformGetDefaultVideoOutputsConfiguration(QBVideoOutputConfig *cfg,
                                              size_t count,
                                              QBTVSystem system);

/**
 * Get name of the PAL system
 *
 * @param[in] palSystem     PAL system
 * @return                  printable name of the PAL system @a palSystem
 **/
extern const char *
QBPlatformGetPALSystemName(QBPALSystem palSystem);

/**
 * Get last reset reason.
 *
 * @return                  @c reason of last system reset
 **/
extern QBPlatformResetReason
QBPlatformGetResetReason(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif


#endif

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

#ifndef QB_PLATFORM_RF_OUTPUT_H_
#define QB_PLATFORM_RF_OUTPUT_H_

/**
 * @file QBPlatformRFOutput.h RF output control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformRFOutput RF output control
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Get list of channel numbers supported by RF output.
 *
 * @param[in] outputID  video output ID of the RF output
 * @param[out] channels array filled with channel numbers
 * @param[in] maxChannels number of elements in @a channels array
 * @return              number of supported channels (can be higher
 *                      than @a maxChannels), @c -1 in case of error
 **/
extern ssize_t
QBPlatformRFOutputGetSupportedChannels(unsigned int outputID,
                                       unsigned int *channels,
                                       size_t maxChannels);

/**
 * Set channel number used by RF output.
 *
 * @param[in] outputID  video output ID of the RF output
 * @param[in] channel   channel number
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformRFOutputSetChannelNumber(unsigned int outputID,
                                   unsigned int channel);

/**
 * Get channel number used by RF output.
 *
 * This function reads the RF channel currently used by the RF output.  If RF output is disabled, the result of this
 * function is undefined.  If reading the channel is not supported or if an error occurs, the result is zero.
 *
 * @param[in] outputID            video output ID of the RF output
 * @param[out] channelNumber    current channel number
 * @return                      @c 0 on success, @c -1 in case of error
**/
extern int
QBPlatformRFOutputGetChannelNumber(unsigned int outputID,
                                   unsigned int *channelNumber);

/**
 * Check if QBPlatformRFOutputSetChannelNumber can be used to set RF modulator output channel.
 *
 * @param[in]  outputID                video output ID of the RF output
 * @param[out] setChannelAllowed     true if RF channel QBPlatformRFOutputSetChannelNumber can be used
 * @return                           @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformRFSetChannelNumberIsAllowed(unsigned int outputID, bool *setChannelAllowed);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

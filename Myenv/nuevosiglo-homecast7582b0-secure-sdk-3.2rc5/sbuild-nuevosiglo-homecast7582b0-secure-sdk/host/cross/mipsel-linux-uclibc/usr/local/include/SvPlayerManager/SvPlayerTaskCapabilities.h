/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_PLAYER_TASK_CAPABILITIES_H_
#define SV_PLAYER_TASK_CAPABILITIES_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup SvPlayerTask
 * @{
 **/

typedef enum {
    SvPlayerTaskCapability_unknown = -1,
    SvPlayerTaskCapability_no = 0,
    SvPlayerTaskCapability_yes = 1,
} SvPlayerTaskCapability;

typedef struct {
    SvPlayerTaskCapability canPause;
    SvPlayerTaskCapability canTrickplay;
    SvPlayerTaskCapability canSeek;
    SvPlayerTaskCapability canRecord;
    SvPlayerTaskCapability remote_timeshift;
} SvPlayerTaskCapabilities;

static inline SvPlayerTaskCapabilities SvPlayerTaskCapabilitiesNone(void)
{
    SvPlayerTaskCapabilities caps;
    caps.canPause = SvPlayerTaskCapability_no;
    caps.canTrickplay = SvPlayerTaskCapability_no;
    caps.canSeek = SvPlayerTaskCapability_no;
    caps.canRecord = SvPlayerTaskCapability_no;
    caps.remote_timeshift = SvPlayerTaskCapability_no;
    return caps;
}

static inline SvPlayerTaskCapabilities SvPlayerTaskCapabilitiesUnknown(void)
{
    SvPlayerTaskCapabilities caps;
    caps.canPause = SvPlayerTaskCapability_unknown;
    caps.canTrickplay = SvPlayerTaskCapability_unknown;
    caps.canSeek = SvPlayerTaskCapability_unknown;
    caps.canRecord = SvPlayerTaskCapability_unknown;
    caps.remote_timeshift = SvPlayerTaskCapability_unknown;
    return caps;
}

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

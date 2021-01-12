/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SVCONTENTVARIANT_H_
#define SVCONTENTVARIANT_H_

//----------------------------------------------------
// Events

/// Param is: (optional) SvArray variants - of SvContentVariant
#define PLAYER_EVENT_CONTENT_VARIANTS_AVAILABLE   "content:variants_available"

/// Param is: bitrate of the selected variant
#define PLAYER_EVENT_CONTENT_NEW_BITRATE_SELECTED   "content:new_bitrate_selected"


//----------------------------------------------------
// Opts

struct player_opt_content_switch_variant
{
    int selectedVariant;        // one of the variants that we got from player event ^^^
};

/// Usage:
///     int res = SvPlayerTaskSetOpt(g_player_task, PLAYER_OPT_CONTENT_SWITCH_VARIANT, &variant);
///     // res = 0 - newVariant = currentVariant
///     // res > 0 - variant changed
///     if (res > 0) {      // we have to set speed and position in the new variant
///         SvPlayerTaskState state = SvPlayerTaskGetState(g_player_task);
///         SvPlayerTaskPlay(g_player_task, 1.0, state.currentPosition, NULL);    // for now new variant is started
///                                                                               // automatically
///     }
#define PLAYER_OPT_CONTENT_SWITCH_VARIANT   "content:switch_variant"



struct player_opt_content_select_bitrate
{
    int newBitrate;        // set new bitrate to the previous bitrate
};

/// Usage:
///     int res = SvPlayerTaskSetOpt(g_player_task, PLAYER_OPT_CONTENT_SELECT_BITRATE, &bitrate);
///     // res = real new bitrate (<= selected bitrate)
///     if (res > 0) {      // we have to set speed and position in the new variant
///         SvPlayerTaskState state = SvPlayerTaskGetState(g_player_task);
///         SvPlayerTaskPlay(g_player_task, 1.0, state.currentPosition, NULL);    // for now new variant is started
///                                                                               // automatically
///     }
#define PLAYER_OPT_CONTENT_SELECT_BITRATE   "content:select_bitrate"

//----------------------------------------------------
// Data types

/**
 * @file SvContentVariant.h SvContentVariant class API
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

/**
 * @defgroup SvContentVariant The SvContentVariant class.
 * @ingroup SvPlayerKit_Metadata
 * @{
 **/

/**
 * The SvContentVariant reference type.
 *
 * This type should be used to store references to the instances of the
 * SvContentVariant class or it's sub-classes.
 **/
typedef struct SvContentVariant_ {
    struct SvObject_ super_;
    SvString url;
    int programID;
    int bandwidth;
    int resolutionX;
    int resolutionY;
    SvString resolutionString;
    SvString codecs;
} *SvContentVariant;


extern SvType
SvContentVariant_getType(void);

extern SvContentVariant
SvContentVariantCreate(SvString url,
                       int programID,
                       int bandwidth,
                       int resolutionX,
                       int resolutionY,
                       SvString resolutionString,
                       SvString codecs,
                       SvErrorInfo *errorOut);


/** @} */

#endif /* SVCONTENTVARIANT_H_ */

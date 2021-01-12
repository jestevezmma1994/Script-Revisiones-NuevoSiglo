/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SVPLAYERTASKTRACKCONTROLLERVTABLE_H
#define SVPLAYERTASKTRACKCONTROLLERVTABLE_H

/**
 * @file SvPlayerTrackControllerVTable.h VTable for SvPlayerTrackController
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvPlayerTrackControllerVTable VTable for SvPlayerTrackController class
 * @{
 * Player Track Controller VTable
 */

/**
 * Virtual methods table for Player Controller class.
 **/
typedef struct SvPlayerTrackControllerVTable_ {
    /** virtual methods of the base class */
    struct SvObjectVTable_ super_;

    /**
     * Set decoder
     *
     * @param[in] self Player Track Controller object
     * @param[in] decoder decoder to be set
     */
    void (*setDecoder)(SvPlayerTrackController self, QBDecoder *decoder);

    /**
     * Set current video track
     *
     * @param[in] self Player Track Controller object
     * @param idx index of video track to be set
     */
    void (*setVideoTrack)(SvPlayerTrackController self, unsigned int idx);

    /**
     * Set current audio track
     *
     * @param[in] self Player Track Controller object
     * @param idx index of audio track to be set
     */
    void (*setAudioTrack)(SvPlayerTrackController self, unsigned int idx);

    /**
     * Set current subtitles track
     *
     * @param[in] self Player Track Controller object
     * @param idx index of subtitles track to be set
     */
    void (*setSubsTrack)(SvPlayerTrackController self, unsigned int idx);

} *SvPlayerTrackControllerVTable;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* SVPLAYERTASKTRACKCONTROLLERVTABLE_H */

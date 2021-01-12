/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_LATENS_OSDDISPLAYER_H
#define QB_LATENS_OSDDISPLAYER_H

#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBLatensOSDDisplayer Latens OSD stack displayer
 * @ingroup QBLatens
 * @{
 */

/**
 * @file QBLatensOSDDisplayer.h QBLatensOSDDisplayer interface file
 */


/**
 * @enum QBLatensOSDTextType
 * @brief Describes OSD text type
 */
typedef enum {
    QBLatensOSDTextType_ssmDownload = 0,
    QBLatensOSDTextType_notSubscirbed,
    QBLatensOSDTextType_ftaBlockingBlock,
    QBLatensOSDTextType_notAuth,
    QBLatensOSDTextType_ftaBlockingEntBased,
    QBLatensOSDTextType_max,
} QBLatensOSDTextType;

/**
 * @interface QBLatensOSDDisplayer
 * @brief QBLatensOSDDisplayer is used as abstract diplayer of Latens OSD stack.
 * QBLatensOSDDisplayer has to compatible with Latens' OSD requirements.
 */
typedef const struct QBLatensOSDDisplayer_i {
    /**
     * @brief Display OSD state screen.
     *
     * This method is used to notify the OSD displayer set by @ref QBLatensSetOSDDisplayer
     * that a new OSD state screen is to be displayed.
     *
     * @param[in] self_            handle to an object implementing @ref QBLatensOSDDisplayer
     * @param[in] screenHandle     screen handle value of the OSD screen
     * @param[in] textType         text type to be displayed
     * @param[in] blockFTAPlayback flag used for request of  blocking FTA services
     * @return                     handle to an ddobject at index @a idx,
     *                      @c NULL if not available
     **/
    void (*displayStateScreen)(SvObject self_,
                               SvValue screenHandle,
                               QBLatensOSDTextType textType,
                               SvString channel,
                               bool blockFTAPlayback);
    /**
     * @brief Display OSD Remote Message Screen
     *
     * @param self_                  handle to an object implementing @ref QBLatensOSDDisplayer
     * @param screenHandle           screen handle value of the OSD screen
     * @param text                   text message to be displayed
     * @param channel                a URL relating to the channel (eq. "dvb://netid=1:tsid=1:serv=1")
     * @param textColour             text colour in ARGB format
     * @param backgroundColour       background colour in ARGB format
     * @param textTransparency       text transparency, range [0; 100]
     * @param backgroundTransparency background transparency, range [0; 100]
     * @param fontSize               indicates which of 5 font sizes to use. Actual font size is set by @ref QBLatensOSDDisplayer implementation
     * @param forced                 indicates  whether the message is to be displayed as forced to or unforced
     */
    void (*displayRemoteMessageScreen)(SvObject self_,
                                       SvValue screenHandle,
                                       SvString text,
                                       SvString channel,
                                       unsigned long textColour,
                                       unsigned long backgroundColour,
                                       unsigned char textTransparency,
                                       unsigned char backgroundTransparency,
                                       unsigned char fontSize,
                                       unsigned char forced);

    /**
     * @brief Display OSD Overlay Screen
     *
     * @param self_                  handle to an object implementing @ref QBLatensOSDDisplayer
     * @param screenHandle           screen handle value of the OSD screen
     * @param textColour             text colour in ARGB format
     * @param backgroundColour       background colour in ARGB format
     * @param X_Pos                  X position of bottom left corner of the screen given as a percentage of screen size relative to the bottom left of the television screen
     * @param Y_pos                  Y position of bottom left corner of the screen given as a percentage of screen size relative to the bottom left of the television screen
     * @param random                 indicates whether the screen is to be displayed at a random position.
     * @param textTransparency       text transparency, range [0; 100]
     * @param backgroundTransparency background transparency, range [0; 100]
     * @param fontSize               indicates which of 5 font sizes to use. Actual font size is set by @ref QBLatensOSDDisplayer implementation
     * @param covert                 indicates whether the screen should display overtly or covertly (currently covert is not supported)
     * @param text                   text to be displayed
     */
    void (*displayIdOverlay)(SvObject self_,
                             SvValue screenHandle,
                             SvString channel,
                             unsigned long textColour,
                             unsigned long backgroundColour,
                             unsigned char X_Pos,
                             unsigned char Y_Pos,
                             unsigned char random,
                             unsigned char textTransparency,
                             unsigned char backgroundTransparency,
                             unsigned char fontSize,
                             unsigned char covert,
                             SvString text);

    /**
     * @brief Remove OSD State Screen
     *
     * @param self_        handle to an object implementing @ref QBLatensOSDDisplayer
     * @param screenHandle screen handle value of the OSD screen
     * @return             removed screen handle
     */
    int (*removeStateScreen)(SvObject self_, SvValue screenHandle);

    /**
     * @brief Remove OSD Remote Message Screen
     *
     * @param self_        handle to an object implementing @ref QBLatensOSDDisplayer
     * @param screenHandle screen handle value of the OSD screen
     * @return             removed screen handle
     */
    int (*removeRemoteMessageScreen)(SvObject self_, SvValue screenHandle);

    /**
     * @brief Remove OSD Overlay Screen
     *
     * @param self_        handle to an object implementing @ref QBLatensOSDDisplayer
     * @param screenHandle screen handle value of the OSD screen
     * @return             removed screen handle
     */
    int (*removeIdOverlay)(SvObject self_, SvValue screenHandle);
} *QBLatensOSDDisplayer;

/**
 * @brief Get runtime type identification object representing Latens OSD displayer interface.
 *
 * @return QBLatensOSDDisplayer interface object
 */
SvInterface QBLatensOSDDisplayer_getInterface(void);

/** @} */

#endif // QB_LATENS_OSDDISPLAYER_H

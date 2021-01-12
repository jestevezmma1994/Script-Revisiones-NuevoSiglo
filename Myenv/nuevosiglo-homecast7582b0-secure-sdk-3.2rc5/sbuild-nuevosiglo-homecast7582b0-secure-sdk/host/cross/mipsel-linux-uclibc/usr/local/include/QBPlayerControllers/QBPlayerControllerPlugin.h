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

#ifndef QB_PLAYER_CONTROLLER_PLUGIN_H
#define QB_PLAYER_CONTROLLER_PLUGIN_H

/**
 * @file QBPlayerControllerPlugin.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 *
 * @brief Cubiware MW Channel Player Controller Plugins API.
 **/

#include <SvPlayerKit/SvContent.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvCoreTypes.h>
#include <stdbool.h>

/**
 * @defgroup QBPlayerControllerPlugins Cubiware Player Controller Plugin Interface
 * @ingroup QBMWChannelPlayerController
 * @{
 *
 * An interface for Cubiware VoD and channel player controllers plugins.
 **/

/**
 * @brief QBPlayerControllerPluginAnswerHandlingResult UseProduct answer handling results.
 */
typedef enum {
    QBPlayerControllerPluginAnswerHandlingResult_ok = 0,    /**< answer handled successfully */
    QBPlayerControllerPluginAnswerHandlingResult_wait,      /**< answer handled successfully but user needs to wait for addtional data.
                                                                 @see readyCallback for details */
    QBPlayerControllerPluginAnswerHandlingResult_error      /**< failed to handle answer */
} QBPlayerControllerPluginAnswerHandlingResult;

/**
 * @brief QBPlayerControllerPlugin interface.
 */
struct QBPlayerControllerPlugin_ {
    /**
     * Prepare content to playback and report if all necessary data to
     * start playback are ready from the plugin's point of view.
     *
     * @param[in] self_     plugin handle
     * @param[in] content   content that will be played
     * @param[in] data      additional data dependent on plugin type (VoD/Channel)
     * @return              true if plugin has all necessary data
     */
    bool (*prepareContentToPlayback)(SvObject self_, SvContent content, SvObject data);

    /**
     * UseProduct answer handling function.
     *
     * @param[in] self_     plugin handle
     * @param[in] answer    UseProduct answer
     * @param[in] listener  optional plugin listener object - @see QBPlayerControllerPluginListener_
     * @return              answer handling status
     */
    QBPlayerControllerPluginAnswerHandlingResult (*handleAnswer)(SvObject self_, SvHashTable answer, SvObject listener);

    /**
     * Plugin start method.
     * @param[in] self_     plugin handle
     */
    void (*start)(SvObject self_);

    /**
     * Plugin stop method.
     * @param[in] self_     plugin handle
     */
    void (*stop)(SvObject self_);

    /**
     * This fucntion indicates if MW should contact Conax Contego system for authentication data.
     *
     * If license is present UseProduct call will not request contego data.
     *
     * @param[in] self_     plugin handle
     * @return              true if contego call is not needed or false otherwise
     */
    bool (*hasLicense)(SvObject self_);
};

/**
 * @brief QBPlayerControllerPlugin main type.
 * @see QBPlayerControllerPlugin_
 */
typedef struct QBPlayerControllerPlugin_* QBPlayerControllerPlugin;

/**
 * Get runtime type identification object representing QB player controller plugin interface.
 *
 * @return QBPlayerControllerPlugin interface object
 **/
SvInterface QBPlayerControllerPlugin_getInterface(void);

/**
 * QBPlayerControllerPluginListener interface.
 **/
typedef struct QBPlayerControllerPluginListener_ {
    /**
     * Plugin ready callback.
     *
     * This callback is used when handleAnswer plugin method returned wait status.
     * Indicates that plugin's job is done.
     *
     * @param[in] self           listener object
     * @param[in] ready          true on success or false on error
     * @param[in] plugin         plugin that called this callback
     * @param[in] errorMessage   message describing the error
     */
    void (*readyCallback)(SvObject self_, bool ready, SvObject plugin, SvString errorMessage);
}* QBPlayerControllerPluginListener;

/**
 * Get runtime type identification object representing QB player controller plugin listener interface.
 * @return interface object or NULL in case of error
 */
SvInterface QBPlayerControllerPluginListener_getInterface(void);

/**
 * @}
 **/

#endif // QB_PLAYER_CONTROLLER_PLUGIN_H

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
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SVCHANNELMERGER_H
#define SVCHANNELMERGER_H

/**
 * @file SvChannelMerger.h
 * @brief Channel merger interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvFoundation/SvArray.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup SvChannelMergers Mergers for channels comming from different sources
 * @ingroup SvEPGDataLayer
 **/

typedef const struct SvChannelMerger_ {
    /**
     * Set plugin whose channels will be set as current in case of
     * channels having the same IDs.
     *
     * @param[in] self_         Merger handle
     * @param[in] pluginID      plugin ID to be preferred
     * @param[out] errorOut     error info
     */
    void (*setPreferredPluginID)(SvGenericObject self_,
                                 unsigned int pluginID,
                                 SvErrorInfo *errorOut);

    /**
     * Get ID of the plugin whose channels are preferred.
     *
     * @param[in] self_         Merger handle
     * @param[out] errorOut     error info
     * @return                  preferred plugin ID
     */
    unsigned int (*getPreferredPluginID)(SvGenericObject self_,
                                         SvErrorInfo *errorOut);

    /**
     * Add channel plugin.
     *
     * @param[in] self_         Merger handle
     * @param[in] plugin        plugin to be listened
     * @param[out] errorOut     error info
     */
    void (*addPlugin)(SvGenericObject self_,
                      SvGenericObject plugin,
                      SvErrorInfo *errorOut);

    /**
     * Remove channel plugin.
     *
     * @param[in] self_         Merger handle
     * @param[in] pluginID      plugin to be removed
     * @param[out] errorOut     error info
     */
    int (*removePlugin)(SvGenericObject self_,
                        SvGenericObject plugin,
                        SvErrorInfo *errorOut);

    /**
     * Get array of registered plugins.
     *
     * @param[in] self_         Merger handle
     * @param[out] errorOut     error info
     */
    SvArray (*getPlugins)(SvGenericObject self_,
                          SvErrorInfo *errorOut);

    /**
     * Add merger listener.
     *
     * @param[in] self_         Merger handle
     * @param[in] listener      new listener
     * @param[out] errorOut     error info
     */
    void (*addListener)(SvGenericObject self_,
                        SvGenericObject listener,
                        SvErrorInfo *errorOut);

    /**
     * Remove merger listener.
     *
     * @param[in] self_         Merger handle
     * @param[in] listener      listener to be removed
     * @param[out] errorOut     error info
     */
    void (*removeListener)(SvGenericObject self_,
                           SvGenericObject listener,
                           SvErrorInfo *errorOut);

    /**
     * Start channel merger.
     *
     * @param[in] self_         Merger handle
     * @param[in] scheduler     scheduler to be used by merger
     * @param[out] errorOut     error info
     */
    void (*start)(SvGenericObject self_,
                  SvScheduler scheduler,
                  SvErrorInfo *errorOut);

    /**
     * Stop channel merger.
     *
     * @param[in] self_         Merger handle
     * @param[out] errorOut     error info
     */
    void (*stop)(SvGenericObject self_,
                 SvErrorInfo *errorOut);

    /**
     * Propagate notification about change in run-time properties
     * of one of TV channels.
     *
     * @param[in] self_         Merger handle
     * @param[in] channel       changed channel
     * @param[out] errorOut     error info
     */
    void (*propagateChannelChange)(SvGenericObject self_,
                                   SvTVChannel channel,
                                   SvErrorInfo *errorOut);

    /**
     * Check if merger is processing / propagating channels.
     *
     * @param[in] self_         merger handle
     * @param[out] errorOut     error info
     * @return                  @c true if merger is processing/propagating channels
     */
    bool (*isPropagatingChannels)(SvGenericObject self_,
                                  SvErrorInfo* errorOut);

    /**
     * Add channel filter.
     *
     * @param[in] self_         Merger handle
     * @param[in] filter        filter to use while channel merging
     * @param[out] errorOut     error info
     */
    void (*addFilter)(SvObject self_,
                      SvObject plugin,
                      SvErrorInfo *errorOut);
} *SvChannelMerger;

/**
 * Get runtime type identification object representing
 * channel merger interface.
 *
 * @return SvChannelMerger interface object
 **/
extern SvInterface
SvChannelMerger_getInterface(void);

/** @} */

#endif // SVCHANNELMERGER_H

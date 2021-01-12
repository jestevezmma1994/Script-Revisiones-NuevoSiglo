/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

/* App/Libraries/SvEPGDataLayer/Plugins/DVBEPGPlugin.h */

#ifndef SV_EPG_DVB_EPG_PLUGIN_H_
#define SV_EPG_DVB_EPG_PLUGIN_H_

/**
 * @file DVBEPGPlugin.h
 * @brief DVB EPG Plugin Class API.
 **/

/**
 * @defgroup DVBEPGPlugin DVB EPG plugin
 * @ingroup SvEPGPlugins
 **/

/**
 * @defgroup DVBEPGPlugin DVB EPG plugin class
 * @ingroup DVBEPGPlugin
 * @{
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvEPGDataLayer/Plugins/DVBEPGChannel.h>
#include <QBChannelScanningConf.h>


/**
 * DVB EPG plugin class.
 **/
typedef struct DVBEPGPlugin_s *DVBEPGPlugin;

/**
 * DVB EPG plugin update params class.
 **/
struct DVBEPGPluginUpdateParams_s {
    struct SvObject_ super_;
    bool needName;
    bool needLcn;
    bool nameOverwrite;
    bool lcnOverwrite;
};
typedef struct DVBEPGPluginUpdateParams_s DVBEPGPluginUpdateParams;

struct DVBEPGPluginMarkAllUnseenAsDeadParams_s {
    struct SvObject_ super_;
    SvHashTable metadata;
};
typedef struct DVBEPGPluginMarkAllUnseenAsDeadParams_s DVBEPGPluginMarkAllUnseenAsDeadParams;

typedef void (*DVBEPGChannelFill)(void *self_, DVBEPGPlugin plug, DVBEPGChannel channel);
typedef bool (*DVBEPGPluginCanRemoveVariant)(void *self_, DVBEPGPluginMarkAllUnseenAsDeadParams* params, DVBEPGChannel channel, DVBEPGChannelVariant variant, bool remove);


/**
 * Get runtime type identification object
 * representing DVB EPG plugin class.
 **/
extern SvType
DVBEPGPlugin_getType(void);

/**
 * Initialize DVBEPGPlugin instance.
 *
 * @param[in] self      DVB EPG plugin handle
 * @param[out] errorOut error info
 * @return              self or NULL in case of error
 **/
extern DVBEPGPlugin
DVBEPGPluginInit(DVBEPGPlugin self,
                 SvErrorInfo *errorOut);

void
DVBEPGPluginSetCanRemoveVariant(DVBEPGPlugin plug, DVBEPGPluginCanRemoveVariant canRemoveVariant, void *canRemoveVariantData);


/**
 * Mark all channels as "unseen".
 * Channels can be updated with DVBEPGPluginUpdate() later.
 * Then, every channel that has not been updated, can be removed with DVBEPGPluginDropUnseen().
 */
extern void
DVBEPGPluginStartUpdate(DVBEPGPlugin plug);

/**
 * Add new channels and update metadata of existing channels, according scannign results given with \a array.
 * @param[in] array  array of QBDVBChannelDesc
 */
extern void
DVBEPGPluginUpdate(DVBEPGPlugin plug, SvArray channels, SvHashTable networks, SvHashTable muxes, DVBEPGPluginUpdateParams* params);

/**
 * All channels that are still marked as "unseen" (were not updated since last call to DVBEPGPluginStartUpdate()),
 * will be marked as "dead".
 * @param[in] checkedFreqCnt  size of @a checkedFreqTab
 * @param[in] checkedFreqTab  array of frequencies to check, will be freed
 * @param[in] params          object describing metadata for this operation
 */
extern void
DVBEPGPluginMarkUnseenAsDead(DVBEPGPlugin plug, int checkedFreqCnt, int* checkedFreqTab, DVBEPGPluginMarkAllUnseenAsDeadParams* params);

/*
 * Forget all channels
 */
void DVBEPGPluginDropAllChannels(DVBEPGPlugin plug);

void DVBEPGPluginIgnoreTsid(DVBEPGPlugin plug, bool ignore);

SvString DVBEPGPluginGenerateChannelId(DVBEPGPlugin plug, int onid, int sid, int tsid);

void DVBEPGPluginAddChannelNumberMerger(DVBEPGPlugin plug, SvGenericObject numberMerger);

DVBEPGPluginUpdateParams* DVBEPGPluginCreateUpdateParams(DVBEPGPlugin plugin);
void DVBEPGPluginUpdateParamsFillFromScanningConf(DVBEPGPluginUpdateParams* params, QBChannelScanningConf conf);

DVBEPGPluginMarkAllUnseenAsDeadParams* DVBEPGPluginCreateMarkAllUnseenAsDeadParams(DVBEPGPlugin plugin);

/**
 * @}
 **/

#endif

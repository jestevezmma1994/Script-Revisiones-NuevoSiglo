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

#include "SIDAsChannelNumberPlugin.h"

#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvPlayerKit/SvChbuf.h>
#include <mpeg_tables/nit.h>
#include <QBDVBScannerPlugin.h>
#include <QBDVBScannerTypes.h>


struct SIDAsChannelNumberPlugin_ {
    struct SvObject_ super_;
};

SvLocal bool
SIDAsChannelNumberPluginIsReady(SvObject self_, QBDVBScanner *scanner)
{
    return true;
}

SvLocal void
SIDAsChannelNumberPluginUpdateSDTChannel(SvObject self_,
                                         QBDVBChannelDesc channel,
                                         QBDVBChannelVariant *variant,
                                         mpeg_sdt_element *SDTElement,
                                         struct base_results_s *base,
                                         char **name,
                                         char **provider,
                                         int *running,
                                         bool *freeCAMode,
                                         SvHashTable metadata)
{
}

SvLocal void
SIDAsChannelNumberPluginUpdateNITChannel(SvObject self_,
                                         QBDVBChannelDesc channel,
                                         QBDVBChannelVariant *variant,
                                         mpeg_nit_element *NITElement,
                                         struct base_results_s *base,
                                         int *LCN,
                                         int *visible,
                                         int *NID,
                                         SvHashTable metadata,
                                         QBTunerMuxIdObj *actualMuxId)
{
}

SvLocal void
SIDAsChannelNumberPluginAfterSDTChannelUpdate(SvObject self_,
                                              QBDVBChannelDesc channel,
                                              QBDVBChannelVariant *variant,
                                              mpeg_sdt_element *SDTElement,
                                              SvHashTable metadata)
{
    if (channel->lcn < 0)
        channel->lcn = mpeg_sdt_element_get_sid(SDTElement);
}

SvLocal void
SIDAsChannelNumberPluginAfterNITChannelUpdate(SvObject self_,
                                              QBDVBChannelDesc channel,
                                              QBDVBChannelVariant *variant,
                                              mpeg_nit_element *NITElement,
                                              SvHashTable metadata,
                                              QBTunerMuxIdObj *actualMuxId)
{
}

SvLocal void
SIDAsChannelNumberPluginReceivedNIT(SvObject self_,
                                    QBDVBScanner *scanner,
                                    SvChbuf chb,
                                    SvHashTable metadata,
                                    QBTunerMuxIdObj *actualMuxId)
{
}

SvLocal void
SIDAsChannelNumberPluginReceivedPAT(SvObject self_,
                                    QBDVBScanner *scanner,
                                    SvChbuf chb,
                                    SvHashTable metadata)
{
}

SvLocal SvType
SIDAsChannelNumberPlugin_getType(void)
{
    static const struct QBDVBScannerPlugin_ pluginMethods = {
        .isReady               = SIDAsChannelNumberPluginIsReady,
        .updateSDTChannel      = SIDAsChannelNumberPluginUpdateSDTChannel,
        .updateNITChannel      = SIDAsChannelNumberPluginUpdateNITChannel,
        .afterUpdateSDTChannel = SIDAsChannelNumberPluginAfterSDTChannelUpdate,
        .afterUpdateNITChannel = SIDAsChannelNumberPluginAfterNITChannelUpdate,
        .receivedNIT           = SIDAsChannelNumberPluginReceivedNIT,
        .receivedPAT           = SIDAsChannelNumberPluginReceivedPAT
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("SIDAsChannelNumberPlugin",
                            sizeof(struct SIDAsChannelNumberPlugin_),
                            SvObject_getType(),
                            &type,
                            QBDVBScannerPlugin_getInterface(), &pluginMethods,
                            NULL);
    }

    return type;
}

SIDAsChannelNumberPlugin
SIDAsChannelNumberPluginCreate(void)
{
    return (SIDAsChannelNumberPlugin) SvTypeAllocateInstance(SIDAsChannelNumberPlugin_getType(), NULL);
}

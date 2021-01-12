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

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvBinaryTree.h>
#include <mpeg_descriptors.h>
#include <mpeg_tables/sdt.h>
#include <mpeg_tables/bat.h>
#include <mpeg_descriptors/mpeg/name.h>
#include <mpeg_descriptors/dvb/linkage.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <SvEPGDataLayer/Plugins/DVBEPGChannel.h>
#include <SvEPGDataLayer/Plugins/DVBEPGPlugin.h>
#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <QBDVBLinkage.h>
#include <main.h>
#include "sdtMonitor.h"

struct QBSDTMonitor_t {
    struct SvObject_ super_;

    DVBEPGChannel key;

    SvBinaryTree channels;

    SvGenericObject plugin;

    QBDVBTableMonitor tableMonitor;
};

SvLocal void QBSDTMonitorSDTTable(QBSDTMonitor self, mpeg_sdt_parser* parser)
{
    int tsid = mpeg_sdt_parser_get_tsid(parser);
    int onid = mpeg_sdt_parser_get_onid(parser);

    list_t *list = mpeg_sdt_parser_get_service_list(parser);
    mpeg_sdt_element* sdt_element = NULL;
    list_linkage_t *n1, *n2;

    list__for_each_safe(list, sdt_element, n1, n2) {
        int sid = mpeg_sdt_element_get_sid(sdt_element);
        bool running = (mpeg_sdt_element_get_running_status(sdt_element) == 4);

        self->key->sid = sid;
        self->key->tsid = tsid;
        self->key->onid = onid;

        DVBEPGChannel channel = (DVBEPGChannel) SvBinaryTreeGetObject(self->channels, (SvGenericObject) self->key);
        if (!channel) {
            //We should remember, that this channel is running/has some specific linkage
            continue;
        }

        bool was_changed = false;

        if (running != channel->running) {
            channel->running = running;
            was_changed = true;
        }

        SvArray linkages = NULL;

        mpeg_descriptor_map* descriptor_map = mpeg_sdt_element_get_descriptor_map(sdt_element);
        list_t* linkage_list = mpeg_descriptor_map_get_list(descriptor_map, DVB_DESC_TAG_LINKAGE);
        if(linkage_list) {
            mpeg_descriptor* descriptor_iter = NULL;
            list_linkage_t *l1;
            list__for_each(linkage_list, descriptor_iter, l1) {
                dvb_linkage_desc linkage;
                int res = dvb_linkage_desc_init(&linkage, &descriptor_iter->data);
                if (res != 0)
                    continue;

                QBDVBLinkage likage =  QBDVBLinkageCreate(linkage.sid, linkage.tsid, linkage.onid, linkage.type);
                if (!linkages)
                    linkages = SvArrayCreate(NULL);
                if (SvArrayIndexOfObject(linkages, (SvObject) likage) == -1)
                    SvArrayAddObject(linkages, (SvObject) likage);
                SVRELEASE(likage);
                dvb_linkage_desc_destroy(&linkage);
            }
        }

        SvArray savedLinkage = channel->metadata ? (SvArray) SvHashTableFind(channel->metadata, (SvObject) SVSTRING("dvb_linkage")) : NULL;
        if (!SvObjectEquals((SvObject) savedLinkage, (SvObject) linkages)) {
            if (linkages) {
                if (!channel->metadata)
                    channel->metadata = SvHashTableCreate(11, NULL);
                SvHashTableInsert(channel->metadata, (SvGenericObject) SVSTRING("dvb_linkage"), (SvGenericObject) linkages);
            } else if (channel->metadata)
                SvHashTableRemove(channel->metadata, (SvGenericObject) SVSTRING("dvb_linkage"));
            was_changed = true;
        }

        if (was_changed)
            SvInvokeInterface(SvEPGChannelListPlugin, self->plugin, propagateChannelChange, (SvTVChannel) channel, NULL);
        SVTESTRELEASE(linkages);
    }
}


SvLocal void nit_changed(SvGenericObject self, unsigned int tunerNum, mpeg_nit_parser *parser)
{
}

SvLocal void sdt_changed(SvGenericObject self_, unsigned int tunerNum, mpeg_sdt_parser *parser)
{
    QBSDTMonitor self = (QBSDTMonitor) self_;
    if (mpeg_sdt_parser_get_table_id(parser) == MPEG_TABLE_ID_SDT_ACTUAL)
        QBSDTMonitorSDTTable(self, parser);
}

SvLocal void cat_changed(SvGenericObject self, unsigned int tunerNum, mpeg_cat_parser *parser)
{
}

SvLocal void pat_changed(SvGenericObject self, unsigned int tunerNum, mpeg_pat_parser *parser)
{
}

SvLocal void bat_changed(SvObject self, unsigned int tunerNum, mpeg_bat_parser *parser)
{
}

SvLocal void table_repeated(SvGenericObject self_, unsigned int tunerNum, SvBuf chb, QBDVBTableMonitorTableID id)
{
}


SvLocal void QBSDTMonitorChannelFound(SvGenericObject self_, SvTVChannel channel)
{
    QBSDTMonitor self = (QBSDTMonitor) self_;
    SvBinaryTreeInsert(self->channels, (SvGenericObject) channel);
}

SvLocal void QBSDTMonitorChannelModified(SvGenericObject self_, SvTVChannel channel)
{
}

SvLocal void QBSDTMonitorChannelLost(SvGenericObject self_, SvTVChannel channel)
{
    QBSDTMonitor self = (QBSDTMonitor) self_;
    SvBinaryTreeRemove(self->channels, (SvGenericObject) channel);
}

SvLocal void QBSDTMonitorChannelListCompleted(SvGenericObject self_, int pluginID)
{
}

SvLocal void QBSDTMonitor__dtor__(void *self_)
{
    QBSDTMonitor self = self_;
    SVRELEASE(self->key);
    SVRELEASE(self->channels);
    SVRELEASE(self->plugin);
}

SvLocal SvType QBSDTMonitor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSDTMonitor__dtor__
    };
    static SvType type = NULL;
    static const struct SvEPGChannelListListener_ listMethods = {
        .channelFound         = QBSDTMonitorChannelFound,
        .channelModified      = QBSDTMonitorChannelModified,
        .channelLost          = QBSDTMonitorChannelLost,
        .channelListCompleted = QBSDTMonitorChannelListCompleted,
    };
    static const struct QBDVBTableMonitorListener_t tableMonitorMethods = {
        .NITVersionChanged = nit_changed,
        .SDTVersionChanged = sdt_changed,
        .CATVersionChanged = cat_changed,
        .PATVersionChanged = pat_changed,
        .BATVersionChanged = bat_changed,
        .tableRepeated = table_repeated
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSDTMonitor",
                            sizeof(struct QBSDTMonitor_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvEPGChannelListListener_getInterface(), &listMethods,
                            QBDVBTableMonitorListener_getInterface(), &tableMonitorMethods,
                            NULL);
    }

    return type;
}

SvLocal int QBSDTMonitorCompareChannels(void *prv, SvGenericObject objA, SvGenericObject objB)
{
    if (objA == objB)
        return 0;
    DVBEPGChannel chan1 = (DVBEPGChannel) objA;
    DVBEPGChannel chan2 = (DVBEPGChannel) objB;

    if (chan1->sid == chan2->sid) {
        if (chan1->tsid == chan2->tsid)
            return chan1->onid - chan2->onid;
        else
            return chan1->tsid - chan2->tsid;
    } else
        return chan1->sid - chan2->sid;
}

QBSDTMonitor QBSDTMonitorCreate(SvObject plugin)
{
    if (!SvObjectIsInstanceOf(plugin, DVBEPGPlugin_getType())) {
        SvLogError("%s: unsupported plugin", __func__);
        return NULL;
    }

    QBSDTMonitor self = (QBSDTMonitor) SvTypeAllocateInstance(QBSDTMonitor_getType(), NULL);
    self->channels = SvBinaryTreeCreateWithCompareFn(QBSDTMonitorCompareChannels, NULL, NULL);
    self->key = DVBEPGChannelCreate(-1, -1, -1, -1);
    self->plugin = SVRETAIN(plugin);

    SvInvokeInterface(SvEPGChannelListPlugin, plugin, addListener, (SvGenericObject) self, false, NULL);


    return self;
}

void QBSDTMonitorStart(QBSDTMonitor self)
{
    if( !self )
        return;

    self->tableMonitor =
        SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBDVBTableMonitor")));
    if (!self->tableMonitor) {
        SvLogError("Error: %s() [%s:%d]\nDVB table monitor is NULL.", __func__, __FILE__, __LINE__);
        return;
    }

    QBDVBTableMonitorAddListener(self->tableMonitor, (SvObject) self);
}

void QBSDTMonitorStop(QBSDTMonitor self)
{
    if( !self )
        return;

    if (!self->tableMonitor) {
        SvLogError("Error: %s() [%s:%d]\nDVB table monitor is NULL.", __func__, __FILE__, __LINE__);
        return;
    }

    QBDVBTableMonitorRemoveListener(self->tableMonitor, (SvObject) self);
    SVRELEASE(self->tableMonitor);
    self->tableMonitor = NULL;
}


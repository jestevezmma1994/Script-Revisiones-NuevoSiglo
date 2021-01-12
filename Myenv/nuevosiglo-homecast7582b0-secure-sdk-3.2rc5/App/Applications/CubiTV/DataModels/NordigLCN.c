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

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvBinaryTree.h>
#include <QBTuner.h>
#include <QBTunerInfo.h>
#include <QBTunerTypes.h>
#include <SvEPGDataLayer/Plugins/DVBEPGChannel.h>
#include <SvEPGDataLayer/Plugins/DVBEPGPlugin.h>
#include <SvEPGDataLayer/Plugins/Mergers/SvEPGChannelNumberMerger.h>
#include <QBDVBNordigLCN.h>
#include <QBDVBLinkage.h>
#include "NordigLCN.h"

struct QBNordigLCNNode_t {
    struct SvObject_ super_;

    int lcn;
    int lcn2;

    int number;

    SvTVChannel channel;
    QBDVBChannelType type;

    struct QBTunerMuxId muxid;
};
typedef struct QBNordigLCNNode_t* QBNordigLCNNode;

struct QBNordigLCN_t {
    struct SvObject_ super_;

    SvBinaryTree nodeByNumber;

    SvBinaryTree channelsWithNoLCN;
    SvBinaryTree duplicate;
    SvBinaryTree channelsWithLCN;

    SvHashTable nodesByLCN;

    SvHashTable idToNode;

    SvBinaryTree nonPrimaryOnid;

    SvHashTable channelToReplacement;
    SvHashTable replacementToChannel;

    QBNordigLCNNode key;

    SvString filter;

    DVBEPGPlugin plugin; //not retained, we are owned by that plugin

    int primary_onid;
    bool use_primary_onid;

    int listid;
    SvString currentCountry;

    SvHashTable listidToChannels;
    SvHashTable channelToListids;
    SvHashTable listidToCountry;

    SvHashTable lcn2MuxToChannels;
    SvHashTable muxToChannels;

    SvHashTable lcnChanges;

    // minimum number allowed for channels without set LCN number
    int startingNumberForLCNlessChannels;
};

SvLocal SvType
QBNordigLCNNode_getType(void);

SvLocal void QBNordigLCNChannelFound(SvGenericObject self_, SvTVChannel channel);
SvLocal void QBNordigLCNChannelRemoved(SvGenericObject self_, SvTVChannel channel);
SvLocal void QBNordigLCNChannelModified(SvGenericObject self_, SvTVChannel channel);

SvLocal int QBNordigLCNNodeGetLCN(QBNordigLCNNode self)
{
    if (self->lcn2 != -1)
        return self->lcn2;
    return self->lcn;
}
SvLocal void QBNordigLCNGetBasicInfo(QBNordigLCN self, DVBEPGChannel channel, bool* has_lcn, bool *has_lcn2, int *lcn2, bool *lcn2_visible);
SvLocal SvValue QBNordigLCNGetReplacementID(QBNordigLCN self, DVBEPGChannel channel);

SvLocal void QBNordigLCNVerify(QBNordigLCN self)
{
    return;
/*    SvLogError("^^^");

    for (unsigned i = 0; i < SvBinaryTreeGetNodesCount(self->nodeByNumber); i++) {
        QBNordigLCNNode node =  (QBNordigLCNNode)SvBinaryTreeGetObjectAtIndex(self->nodeByNumber, i);
        DVBEPGChannel ch = (DVBEPGChannel) node->channel;
        SvLogError("(%i,%i,%i) %i (%i,%i)", ch->sid, ch->tsid, ch->onid, node->number, node->lcn, node->lcn2);
    }
    SvLogError("vvv");*/

#ifndef NDEBUG
    unsigned channels_count = SvHashTableCount(self->idToNode);
    unsigned duplicate_count = SvBinaryTreeGetNodesCount(self->duplicate);
    unsigned nolcn_channels_count = SvBinaryTreeGetNodesCount(self->channelsWithNoLCN);
    unsigned non_primary_count = SvBinaryTreeGetNodesCount(self->nonPrimaryOnid);
    unsigned lcn_channels_count = SvBinaryTreeGetNodesCount(self->channelsWithLCN);
    SvLogError("channels_count = %i, duplicate_count = %i, nolcn_channels_count = %i, non_primary_count = %i, lcn_channels_count = %i", channels_count, duplicate_count, nolcn_channels_count, non_primary_count, lcn_channels_count);
    assert(lcn_channels_count + duplicate_count + nolcn_channels_count + non_primary_count == channels_count);
    assert(channels_count == SvBinaryTreeGetNodesCount(self->nodeByNumber));
    assert(duplicate_count + nolcn_channels_count + non_primary_count <= channels_count);
    assert(SvHashTableCount(self->lcn2MuxToChannels) <= SvHashTableCount(self->muxToChannels));
    assert(SvHashTableCount(self->channelToReplacement) == SvHashTableCount(self->replacementToChannel));

    SvIterator it = SvBinaryTreeGetIterator(self->nodeByNumber);
    QBNordigLCNNode node;
    int prevNodeNum = -1;
    while ((node = (QBNordigLCNNode) SvIteratorGetNext(&it))) {
        assert(node->number > prevNodeNum);
        prevNodeNum = node->number;
    }

    QBNordigLCNNode lastLcn = (QBNordigLCNNode) SvBinaryTreeGetObjectAtIndex(self->nodeByNumber, lcn_channels_count - 1);
    for (unsigned i = lcn_channels_count; i < channels_count; i++) {
        node = (QBNordigLCNNode) SvBinaryTreeGetObjectAtIndex(self->nodeByNumber, i);
        int expectedNum = (lastLcn ? lastLcn->number + i - lcn_channels_count + 1 : i + 1);
        assert(node->number == expectedNum);
    }
#endif
}

SvLocal int QBNordigLCNCompareById(void *ptr, SvGenericObject objA, SvGenericObject objB)
{
    QBNordigLCNNode nodeA = (QBNordigLCNNode) objA;
    QBNordigLCNNode nodeB = (QBNordigLCNNode) objB;

    if (nodeA->channel == nodeB->channel)
        return 0;
    return strcmp(
            SvStringCString(SvValueGetString(SvDBObjectGetID((SvDBObject) nodeA->channel))),
            SvStringCString(SvValueGetString(SvDBObjectGetID((SvDBObject) nodeB->channel))));
}

SvLocal int QBNordigLCNCompareDuplicates(void *ptr, SvGenericObject objA, SvGenericObject objB)
{
    QBNordigLCNNode nodeA = (QBNordigLCNNode) objA;
    QBNordigLCNNode nodeB = (QBNordigLCNNode) objB;

    if (nodeA == nodeB)
        return 0;

    bool A_has_lcn2 = (nodeA->lcn2 != -1);
    bool B_has_lcn2 = (nodeB->lcn2 != -1);

    if (A_has_lcn2 == B_has_lcn2) {
        bool hdA = QBDVBChannelTypeIsHDTV(nodeA->type);
        bool hdB = QBDVBChannelTypeIsHDTV(nodeB->type);

        if (hdA == hdB) {
            return QBNordigLCNCompareById(ptr, objA, objB);
        } else if (hdA)
            return -1;
        else
            return 1;

    } else if (A_has_lcn2) {
        return -1;
    } else
        return 1;
}


SvLocal int QBNordigLCNCompareByNumber(void *ptr, SvGenericObject objA, SvGenericObject objB)
{
    QBNordigLCNNode nodeA = (QBNordigLCNNode) objA;
    QBNordigLCNNode nodeB = (QBNordigLCNNode) objB;

    return nodeA->number - nodeB->number;
}

SvLocal void QBNordigLCNNotifyChannel(QBNordigLCN self, QBNordigLCNNode node)
{
    if (!self->lcnChanges)
        self->lcnChanges = SvHashTableCreate(60, NULL);

    SvValue lcnVal = SvValueCreateWithInteger(node->number, NULL);
    SvHashTableInsert(self->lcnChanges, (SvGenericObject) node->channel, (SvGenericObject) lcnVal);
    SVRELEASE(lcnVal);
}

SvLocal void QBNordigLCNNotifyChannels(QBNordigLCN self, size_t start, size_t end)
{
    SvGenericObject node = SvBinaryTreeGetObjectAtIndex(self->nodeByNumber, start);
    SvIterator it = SvBinaryTreeGetLowerBound(self->nodeByNumber, (SvGenericObject) node);
    while ((node = SvIteratorGetNext(&it)) && start <= end) {
        start++;
        QBNordigLCNNotifyChannel(self, (QBNordigLCNNode) node);
    }
}

SvLocal SvValue QBNordigLCNGetReplacementID(QBNordigLCN self, DVBEPGChannel channel)
{
    if (!channel->metadata)
        return NULL;

    SvArray linkageArray = (SvArray) SvHashTableFind(channel->metadata, (SvGenericObject) SVSTRING("dvb_linkage"));
    if (!linkageArray)
        return NULL;

    SvIterator it = SvArrayIterator(linkageArray);
    QBDVBLinkage linkage;
    while ((linkage = (QBDVBLinkage) SvIteratorGetNext(&it))) {
        if (linkage->type != 0x82)
            continue;

        if (linkage->onid != channel->onid)
            continue;

        SvString id = DVBEPGPluginGenerateChannelId(self->plugin, linkage->onid, linkage->sid, linkage->tsid);
        SvValue ret = SvValueCreateWithString(id, NULL);
        SVRELEASE(id);
        if (SvObjectEquals((SvObject) SvDBObjectGetID((SvDBObject) channel), (SvObject) ret)) {
            SVRELEASE(ret);
            return NULL;
        }
        return ret;
    }

    return NULL;
}

SvLocal void QBNordigLCNGetList(DVBEPGChannel channel, SvString mainCountry, int *listid, bool *countryFound, SvString *country)
{
    *listid = INT_MAX;
    *countryFound = false;
    *country = NULL;

    if (!channel->metadata)
        return;

    SvArray lcns = (SvArray) SvHashTableFind(channel->metadata, (SvGenericObject) SVSTRING("nordig_lcn"));
    if (!lcns || SvArrayCount(lcns) == 0)
        return;

    QBDVBNordigLCN nlcn = NULL;
    SvIterator it = SvArrayIterator(lcns);
    while ((nlcn = (QBDVBNordigLCN) SvIteratorGetNext(&it))) {
        bool countryMatches = !strcasecmp(SvStringCString(mainCountry), SvStringCString(nlcn->country));

        if (countryMatches && !*countryFound) {
            //We just found the best country, so ignore all previous
            //listid and use this one
            *listid = nlcn->listid;
            *country = nlcn->country;
        }

        if (countryMatches)
            *countryFound = true;

        if (*listid > nlcn->listid) {
            if ((*countryFound && countryMatches) || !*countryFound)
                *listid = nlcn->listid;
                *country = nlcn->country;
        }
    }
}

SvLocal void QBNordigLCNGetBasicInfo(QBNordigLCN self, DVBEPGChannel channel, bool* has_lcn, bool *has_lcn2, int *lcn2, bool *lcn2_visible)
{
    if (has_lcn) {
        QBTunerMuxIdObj* muxid = QBTunerMuxIdObjCreate(&channel->params.mux_id);
        *has_lcn = (channel->lcn > 0) && !SvHashTableFind(self->lcn2MuxToChannels, (SvGenericObject) muxid);
        SVRELEASE(muxid);
    }

    if (has_lcn2)
        *has_lcn2 = false;
    if (lcn2)
        *lcn2 = -1;
    if (lcn2_visible)
        *lcn2_visible = false;

    if ((has_lcn2 || lcn2 || lcn2_visible) && self->listid != INT_MAX) {

        if (channel->metadata) {
            SvArray lcns = (SvArray) SvHashTableFind(channel->metadata, (SvGenericObject) SVSTRING("nordig_lcn"));
            QBDVBNordigLCN nlcn = NULL;
            if (lcns) {
                SvIterator it = SvArrayIterator(lcns);
                while ((nlcn = (QBDVBNordigLCN) SvIteratorGetNext(&it))) {
                    if (nlcn->listid == self->listid)
                        break;
                }
            }

            if (nlcn) {
                if (lcn2)
                    *lcn2 = nlcn->lcn;
                if (lcn2_visible)
                    *lcn2_visible = nlcn->visible;
                if (has_lcn)
                    *has_lcn2 = true;
            }
        }
    }
}

SvLocal size_t QBNordigLCNInsertNode(SvBinaryTree tree, QBNordigLCNNode node)
{
    SvBinaryTreeInsert(tree, (SvGenericObject) node);
    return SvBinaryTreeGetObjectIndex(tree, (SvGenericObject) node);
}

SvLocal void QBNordigLCNCorrectNumbering(QBNordigLCN self, QBNordigLCNNode fromNode, size_t startLcn)
{
    if (!SvBinaryTreeGetNodesCount(self->nodeByNumber))
        return;

    //we want to update lcn's of all nodes AFTER fromNode
    size_t start = fromNode ? SvBinaryTreeGetObjectIndex(self->nodeByNumber, (SvGenericObject) fromNode) + 1 : 0;

    SvIterator it;
    size_t number = startLcn;
    if (fromNode) {
        it = SvBinaryTreeGetUpperBound(self->nodeByNumber, (SvGenericObject) fromNode);
    } else {
        it = SvBinaryTreeGetIterator(self->nodeByNumber);
    }

    QBNordigLCNNode node;
    //This operation is safe because we increase key by one for
    //each [some_index, items_count-1], this doesn't change any order
    while ((node = (QBNordigLCNNode) SvIteratorGetNext(&it))) {
        node->number = number;
        number++;
    }
    size_t end = SvBinaryTreeGetNodesCount(self->nodeByNumber) - 1;

    if (start <= end) {
        QBNordigLCNNotifyChannels(self, start, end);
    }
}

SvLocal int QBNordigLCNInsertLCN(QBNordigLCN self, QBNordigLCNNode node)
{
    if (node->lcn == -1 && node->lcn2 == -1)
        return -1;

    SVAUTOINTVALUE(lcnVal, QBNordigLCNNodeGetLCN(node));
    SvBinaryTree lcns = (SvBinaryTree) SvHashTableFind(self->nodesByLCN, (SvGenericObject) lcnVal);
    if (!lcns) {
        SvValue vLcn = SvValueCreateWithInteger(QBNordigLCNNodeGetLCN(node), NULL);
        lcns = SvBinaryTreeCreateWithCompareFn(QBNordigLCNCompareDuplicates, NULL, NULL);
        SvHashTableInsert(self->nodesByLCN, (SvObject) vLcn, (SvObject) lcns);
        SVRELEASE(lcns);
        SVRELEASE(vLcn);
    }
    SvBinaryTreeInsert(lcns, (SvGenericObject) node);

    return SvBinaryTreeGetObjectIndex(lcns, (SvGenericObject) node);
}

SvLocal void QBNordigLCNRemoveLCN(QBNordigLCN self, QBNordigLCNNode node)
{
    SVAUTOINTVALUE(lcnVal, QBNordigLCNNodeGetLCN(node));
    SvBinaryTree lcns = (SvBinaryTree) SvHashTableFind(self->nodesByLCN, (SvGenericObject) lcnVal);
    if (!lcns)
        return;

    SvBinaryTreeRemove(lcns, (SvGenericObject) node);
    if (!SvBinaryTreeGetNodesCount(lcns))
        SvHashTableRemove(self->nodesByLCN, (SvGenericObject) lcnVal);
}

SvLocal void QBNordigLCNUnmarkReplacement(QBNordigLCN self, SvTVChannel channel)
{
    SvGenericObject oldReplacement = SvHashTableFind(self->channelToReplacement, (SvGenericObject) channel);

    if (oldReplacement) {
        SvHashTableRemove(self->replacementToChannel, (SvGenericObject) oldReplacement);
        SvHashTableRemove(self->channelToReplacement, (SvGenericObject) channel);
    }
}


SvLocal void QBNordigLCNMarkReplacement(QBNordigLCN self, SvTVChannel channel, SvValue replacement)
{
    QBNordigLCNUnmarkReplacement(self, channel);

    if (!replacement)
        return;

    if (SvHashTableFind(self->replacementToChannel, (SvGenericObject) replacement))
        return;

    SvHashTableInsert(self->replacementToChannel, (SvGenericObject) replacement, (SvGenericObject) channel);
    SvHashTableInsert(self->channelToReplacement, (SvGenericObject) channel, (SvGenericObject) replacement);
}

SvLocal void QBNordigLCNMarkLists(QBNordigLCN self, DVBEPGChannel channel)
{
    if (!channel->metadata)
        return;
    SvArray lcns = (SvArray) SvHashTableFind(channel->metadata, (SvGenericObject) SVSTRING("nordig_lcn"));
    if (!lcns)
        return;

    SvArray listids = SvArrayCreate(NULL);
    SvIterator it = SvArrayIterator(lcns);
    QBDVBNordigLCN nlcn;
    while ((nlcn = (QBDVBNordigLCN) SvIteratorGetNext(&it))) {
        SvValue listid = SvValueCreateWithInteger(nlcn->listid, NULL);
        SvArrayAddObject(listids, (SvGenericObject) listid);
        SVRELEASE(listid);
    }
    SvHashTableInsert(self->channelToListids, (SvGenericObject) channel, (SvGenericObject) listids);

    it = SvArrayIterator(lcns);
    while ((nlcn = (QBDVBNordigLCN) SvIteratorGetNext(&it))) {
        SvValue id = SvValueCreateWithInteger(nlcn->listid, NULL);
        SvHashTable channels = (SvHashTable) SvHashTableFind(self->listidToChannels, (SvGenericObject) id);
        if (!channels) {
            channels = SvHashTableCreate(123, NULL);
            SvHashTableInsert(self->listidToChannels, (SvGenericObject) id, (SvGenericObject) channels);
            SVRELEASE(channels);
        }
        SvHashTableInsert(channels, (SvGenericObject) channel, (SvGenericObject) channel);

        if (!SvHashTableFind(self->listidToCountry, (SvGenericObject) id)) {
            SvHashTableInsert(self->listidToCountry, (SvGenericObject) id, (SvGenericObject) nlcn->country);
        }
        SVRELEASE(id);
    }
    SVRELEASE(listids);
}

SvLocal void QBNordigLCNUnmarkLists(QBNordigLCN self, DVBEPGChannel channel)
{
    SvIterator it = SvHashTableKeysIterator(self->listidToChannels);
    SvValue listid;
    while ((listid = (SvValue) SvIteratorGetNext(&it))) {
        SvHashTable channels = (SvHashTable) SvHashTableFind(self->listidToChannels, (SvGenericObject) listid);
        SvHashTableRemove(channels, (SvGenericObject) channel);
        if (SvHashTableCount(channels) == 0) {
            SvHashTableRemove(self->listidToChannels, (SvGenericObject) listid);
            SvHashTableRemove(self->listidToCountry, (SvGenericObject) listid);
        }
    }

    SvHashTableRemove(self->channelToListids, (SvGenericObject) channel);
}

SvLocal void QBNordigLCNCorrectCountry(QBNordigLCN self) {
    //check if current list has been demoted
    if (self->listid == INT_MAX)
        return;
    SVAUTOINTVALUE(listidVal, self->listid);
    if (SvHashTableFind(self->listidToChannels, (SvGenericObject) listidVal))
        return;

    SVTESTRELEASE(self->currentCountry);
    self->currentCountry = NULL;
    self->listid = INT_MAX;

    //Find best listid
    bool countryFound = false;
    SvIterator it = SvHashTableKeysIterator(self->listidToCountry);
    SvValue bestId = NULL;
    SvValue id;
    while ((id = (SvValue) SvIteratorGetNext(&it))) {
        SvString country = (SvString) SvHashTableFind(self->listidToCountry, (SvGenericObject) id);
        bool countryMatches = !strcasecmp("FIN", SvStringCString(country));
        if (!countryFound && countryMatches) {
            countryFound = true;
            bestId = id;
        }
        if ((countryMatches || !countryFound) && (!bestId || SvValueGetInteger(bestId) > SvValueGetInteger(id))) {
            bestId = id;
        }
    }
    if (bestId) {
        self->listid = SvValueGetInteger(bestId);
        self->currentCountry = SVRETAIN(SvHashTableFind(self->listidToCountry, (SvGenericObject) bestId));

        //we got bestlistid, now everyone on new list needs to know that their lcn2 might have changed
        it = SvHashTableKeysIterator((SvHashTable) SvHashTableFind(self->listidToChannels, (SvGenericObject) bestId));
        SvTVChannel ch;
        while ((ch = (SvTVChannel) SvIteratorGetNext(&it))) {
            QBNordigLCNChannelModified((SvGenericObject) self, ch);
        }
    }
}

SvLocal void QBNordigLCNChannelFound(SvObject self_, SvTVChannel tvChannel)
{
    if (!SvObjectIsInstanceOf((SvObject) tvChannel, DVBEPGChannel_getType()))
        return;

    QBNordigLCN self = (QBNordigLCN) self_;

    if (self->filter) {
        if (tvChannel->isDead)
            return;

        SvValue attrTV = (SvValue) SvTVChannelGetAttribute(tvChannel, self->filter);
        if (!attrTV || !SvValueGetBoolean(attrTV))
            return;
    }

    QBNordigLCNNode node = (QBNordigLCNNode) SvHashTableFind(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) tvChannel));

    if (node) {
        QBNordigLCNChannelModified(self_, tvChannel);
        return;
    }

    DVBEPGChannel channel = (DVBEPGChannel) tvChannel;

    if (!SvBinaryTreeGetNodesCount(self->nodeByNumber)) {
        if (channel->tunerType == QBTunerType_cab)
            self->use_primary_onid = false;
        else
            self->use_primary_onid = true;
    }

    node = (QBNordigLCNNode) SvTypeAllocateInstance(QBNordigLCNNode_getType(), NULL);
    node->channel = SVRETAIN(channel);
    node->type = channel->type;
    node->muxid = channel->params.mux_id;

    //If this channel will be replaced, then just drop it
    SvValue replacement = QBNordigLCNGetReplacementID(self, channel);
    if (replacement) {
        QBNordigLCNMarkReplacement(self, tvChannel, replacement);
        if (SvHashTableFind(self->idToNode, (SvGenericObject) replacement)) {
            SVRELEASE(replacement);
            goto fini;
        }
        SVRELEASE(replacement);
    }

    //Check if this channel is a replacement for currently known channel
    SvTVChannel replaced = (SvTVChannel) SvHashTableFind(self->replacementToChannel, (SvGenericObject) SvDBObjectGetID((SvDBObject)channel));
    if (replaced) {
        QBNordigLCNChannelRemoved(self_, replaced);
    }


    int listid;
    bool countryFound;
    SvString country = NULL;
    bool betterList = false;
    //Check if a better list has appeared
    QBNordigLCNGetList(channel, SVSTRING("FIN"), &listid, &countryFound, &country);
    if (country) {
        if (!SvObjectEquals((SvObject) country, (SvObject) self->currentCountry)) {
            if (!strcasecmp(SvStringCString(country), "FIN")) {
                //we upgraded to FIN
                betterList = true;
            } else if ((!self->currentCountry || strcasecmp(SvStringCString(self->currentCountry), "FIN")) && listid < self->listid) {
                //found one with lower listid, while we had no fin
                betterList = true;
            }
        } else {
            if (listid < self->listid) {
                //country did not change, but listid is lower
                betterList = true;
            }
        }
    }

    QBNordigLCNMarkLists(self, channel);

    if (betterList) {
        SvString back = self->currentCountry;
        self->listid = listid;
        self->currentCountry = SVRETAIN(country);
        SVTESTRELEASE(back);
        SvArray channels = SvArrayCreate(NULL);
        SvIterator it = SvBinaryTreeGetIterator(self->nodeByNumber);
        QBNordigLCNNode n;
        while ((n = (QBNordigLCNNode) SvIteratorGetNext(&it))) {
            SvArrayAddObject(channels, (SvGenericObject) n->channel);
        }
        it = SvArrayIterator(channels);
        SvTVChannel ch;
        while ((ch = (SvTVChannel) SvIteratorGetNext(&it))) {
            if (SvHashTableFind(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) ch)))
                QBNordigLCNChannelModified(self_, ch);
        }
        SVRELEASE(channels);
    }


    int lcn2;
    bool has_lcn;
    bool has_lcn2;
    bool lcn2_visible;
    QBNordigLCNGetBasicInfo(self, channel, &has_lcn, &has_lcn2, &lcn2, &lcn2_visible);

    if ((has_lcn2 && !lcn2_visible) || (!has_lcn2 && !channel->visible))
        goto fini;

    int duplicate_count = SvBinaryTreeGetNodesCount(self->duplicate);
    int nolcn_channels_count = SvBinaryTreeGetNodesCount(self->channelsWithNoLCN);
    //int non_primary_count = SvBinaryTreeGetNodesCount(self->nonPrimaryOnid);
    int lcn_channels_count = SvBinaryTreeGetNodesCount(self->channelsWithLCN);


    if (self->primary_onid == -1 || channel->onid == self->primary_onid || !self->use_primary_onid) {
        QBTunerMuxIdObj* muxid = QBTunerMuxIdObjCreate(&channel->params.mux_id);
        if (has_lcn2) {
            SvHashTable lcn2Mux = (SvHashTable) SvHashTableFind(self->lcn2MuxToChannels, (SvGenericObject) muxid);    //gets modified

            SvHashTable newlcn2Mux = lcn2Mux;
            if (!newlcn2Mux) {
                newlcn2Mux = SvHashTableCreate(123, NULL);
                SvHashTableInsert(self->lcn2MuxToChannels, (SvGenericObject) muxid, (SvGenericObject) newlcn2Mux);
                SVRELEASE(newlcn2Mux);
            }
            SvHashTableInsert(newlcn2Mux, (SvGenericObject) channel, (SvGenericObject) channel);
            SvHashTable muxChannels = (SvHashTable) SvHashTableFind(self->muxToChannels, (SvGenericObject) muxid);

            //if there are channels on this mux, but none of them is a lcn2 one and this channel
            //has lcn2, then we need to mark this mux as a lcn2 one and notify all other that
            //they no longer can have lcn
            if (muxChannels && !lcn2Mux) {
                SvIterator it = SvHashTableKeysIterator(muxChannels);
                SvTVChannel ch;
                while ((ch = (SvTVChannel) SvIteratorGetNext(&it))) {
                    SvInvokeInterface(SvEPGChannelNumberMerger, self, mergeOnModified, ch);
                }
            }
        }

        SvHashTable channels = (SvHashTable) SvHashTableFind(self->muxToChannels, (SvGenericObject) muxid);
        if (!channels) {
            channels = SvHashTableCreate(123, NULL);
            SvHashTableInsert(self->muxToChannels, (SvGenericObject) muxid, (SvGenericObject) channels);
            SVRELEASE(channels);
        }
        SvHashTableInsert(channels, (SvGenericObject) channel, (SvGenericObject) channel);
        SVRELEASE(muxid);

        node->lcn = has_lcn ? channel->lcn : -1;
        node->lcn2 = has_lcn2 ? lcn2 : -1;
        node->number = node->lcn2 != -1 ? node->lcn2 : node->lcn;

        int dupIdx = QBNordigLCNInsertLCN(self, node);

        if (!has_lcn && !has_lcn2) {
            int idx = QBNordigLCNInsertNode(self->channelsWithNoLCN, node);
            int prevNodeIdx = lcn_channels_count + idx - 1;
            QBNordigLCNNode prevNode = NULL;
            if (prevNodeIdx >= 0)
                prevNode = (QBNordigLCNNode) SvBinaryTreeGetObjectAtIndex(self->nodeByNumber, prevNodeIdx);

            int num = self->startingNumberForLCNlessChannels - 1;
            if (prevNode && prevNode->number > num) {
                num = prevNode->number;
            }
            QBNordigLCNCorrectNumbering(self, prevNode, num + 2);

            SvHashTableInsert(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) channel), (SvGenericObject) node);
            node->number = num + 1;
            idx = QBNordigLCNInsertNode(self->nodeByNumber, node);
            QBNordigLCNNotifyChannel(self, node);
        } else {
            QBNordigLCNNode dupNode = NULL;
            SVAUTOINTVALUE(lcnVal, QBNordigLCNNodeGetLCN(node));
            SvBinaryTree lcns = (SvBinaryTree) SvHashTableFind(self->nodesByLCN, (SvGenericObject) lcnVal);
            if (dupIdx == 0 && SvBinaryTreeGetNodesCount(lcns) > 1) {
                dupNode = (QBNordigLCNNode) SvBinaryTreeGetObject(self->nodeByNumber, (SvGenericObject) node);
                SVRETAIN(dupNode);
            }

            //The order of following operations is very important, as it allows lcn_channels_count to be constant
            //between uses

            //Ok, so now, we need to deal with duplicates if dupNode != NULL or dupIdx > 0
            //So, if there is a dupNode it means that we need to revoke it
            if (dupNode) {
                SvHashTableRemove(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject)dupNode->channel));
                SvBinaryTreeRemove(self->nodeByNumber, (SvGenericObject) dupNode);
                SvBinaryTreeRemove(self->channelsWithLCN, (SvGenericObject) dupNode);
            }

            //If node is not a duplicate or is the most important one
            //then we need to simply insert it
            if (dupIdx == 0) {
                //Find the last channel in channelsWithLCN
                QBNordigLCNNode lastLCNNode = (QBNordigLCNNode) SvBinaryTreeGetObjectAtIndex(self->channelsWithLCN, SvBinaryTreeGetNodesCount(self->channelsWithLCN) - 1);
                //We did not insert our node yet, so maybe our lcn is the highest
                int lastLCNNumber = lastLCNNode ? lastLCNNode->number : -1;
                if (node->number > lastLCNNumber)
                    lastLCNNumber = node->number;

                int numberOfFirstChannelOutsideLCNRange = (lastLCNNumber >= self->startingNumberForLCNlessChannels) ? lastLCNNumber + 1 : self->startingNumberForLCNlessChannels;

                QBNordigLCNCorrectNumbering(self, lastLCNNode, numberOfFirstChannelOutsideLCNRange);
                SvHashTableInsert(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) channel), (SvGenericObject) node);
                QBNordigLCNInsertNode(self->channelsWithLCN, node);
                QBNordigLCNInsertNode(self->nodeByNumber, node);
                QBNordigLCNNotifyChannel(self, node);
            }

            //If we did not insert node at lcns place we need to to this now.
            //or, if we removed dupNode, it needs to be inserted now
            if (dupIdx > 0 || dupNode) {
                QBNordigLCNNode nodeToInsert = (dupIdx > 0) ? node : dupNode;
                int idx = QBNordigLCNInsertNode(self->duplicate, nodeToInsert);
                int prevNodeIndex = lcn_channels_count + nolcn_channels_count + idx - 1;
                QBNordigLCNNode prevNode = (QBNordigLCNNode) SvBinaryTreeGetObjectAtIndex(self->nodeByNumber, prevNodeIndex);

                int newNumber;
                if ((prevNodeIndex == lcn_channels_count - 1) && (prevNode->number < self->startingNumberForLCNlessChannels)) {
                    // previous channel is last with LCN, we need to ensure that numbering starts at least from base
                    newNumber = self->startingNumberForLCNlessChannels;
                } else {
                    newNumber = prevNode->number + 1;
                }
                QBNordigLCNCorrectNumbering(self, prevNode, newNumber + 1);

                nodeToInsert->number = newNumber;
                SvHashTableInsert(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) nodeToInsert->channel), (SvGenericObject) nodeToInsert);
                idx = QBNordigLCNInsertNode(self->nodeByNumber, nodeToInsert);
                QBNordigLCNNotifyChannel(self, nodeToInsert);
            }
            SVTESTRELEASE(dupNode);
        }
    } else {
        size_t idx = QBNordigLCNInsertNode(self->nonPrimaryOnid, node);
        int prevNodeIndex = lcn_channels_count + nolcn_channels_count + duplicate_count + idx - 1;
        int newNumber;

        QBNordigLCNNode prevNode = NULL;
        if (prevNodeIndex >= 0) {
            prevNode = (QBNordigLCNNode) SvBinaryTreeGetObjectAtIndex(self->nodeByNumber, prevNodeIndex);
            if ((prevNodeIndex == lcn_channels_count - 1) && (prevNode->number < self->startingNumberForLCNlessChannels)) {
              // previous channel is last with LCN, we need to ensure that numbering starts at least from base
              newNumber = self->startingNumberForLCNlessChannels;
            } else {
              newNumber = prevNode->number + 1;
            }
        } else {
          newNumber = self->startingNumberForLCNlessChannels;
        }

        QBNordigLCNCorrectNumbering(self, prevNode, newNumber + 1);

        node->lcn = -1;
        node->lcn2 = -1;
        node->number = prevNode ? prevNode->number + 1 : 1;
        SvHashTableInsert(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) channel), (SvGenericObject) node);
        idx = QBNordigLCNInsertNode(self->nodeByNumber, node);
        QBNordigLCNNotifyChannel(self, node);
    }

fini:
    SVRELEASE(node);
    QBNordigLCNVerify(self);
}

SvLocal void QBNordigLCNChannelModified(SvObject self_, SvTVChannel tvChannel)
{
    if (!SvObjectIsInstanceOf((SvObject) tvChannel, DVBEPGChannel_getType()))
        return;

    QBNordigLCN self = (QBNordigLCN) self_;
    DVBEPGChannel channel = (DVBEPGChannel) tvChannel;
    QBNordigLCNNode node = (QBNordigLCNNode) SvHashTableFind(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) tvChannel));

    if (!node) {
        QBNordigLCNChannelFound(self_, tvChannel);
        return;
    } else if (tvChannel->isDead) {
        QBNordigLCNChannelRemoved(self_, tvChannel);
        return;
    }

    bool has_changed = false;

    int lcn2;
    bool has_lcn;
    bool has_lcn2;
    bool lcn2_visible;
    QBNordigLCNGetBasicInfo(self, channel, &has_lcn, &has_lcn2, &lcn2, &lcn2_visible);
    bool had_lcn = (node->lcn != -1);
    bool had_lcn2 = (node->lcn2 != -1);

    //Here we actualy want to discover whether the channel has changed because
    //a new scan was made or because someone just called propagate channel change

    if ((has_lcn && !had_lcn) || (!has_lcn && had_lcn))
        has_changed = true;
    if ((has_lcn && channel->lcn != node->lcn))
        has_changed = true;

    if ((has_lcn2 && !had_lcn2) || (!has_lcn2 && had_lcn2))
        has_changed = true;
    if ((has_lcn2 && node->lcn2 != lcn2))
        has_changed = true;

    if (has_lcn2 && !lcn2_visible)
        has_changed = true;
    if (!has_lcn2 && !channel->visible)
        has_changed = true;

    if (node->type != channel->type)
        has_changed = true;

    if (!QBTunerMuxIdEqual(&node->muxid, &channel->params.mux_id))
        has_changed = true;

    SvValue replacementID = QBNordigLCNGetReplacementID(self, channel);
    SvValue oldReplacementID = (SvValue) SvHashTableFind(self->channelToReplacement, (SvObject) channel);
    if (!SvObjectEquals((SvObject) replacementID, (SvObject) oldReplacementID))
        has_changed = true;
    SVTESTRELEASE(replacementID);


    SvArray listids = NULL;
    SvArray lcns = NULL;
    if (channel->metadata) {
        lcns = (SvArray) SvHashTableFind(channel->metadata, (SvGenericObject) SVSTRING("nordig_lcn"));
        if (lcns) {
            SvIterator it = SvArrayIterator(lcns);
            QBDVBNordigLCN nlcn;
            while ((nlcn = (QBDVBNordigLCN) SvIteratorGetNext(&it))) {
                SvValue listid = SvValueCreateWithInteger(nlcn->listid, NULL);
                if (!listids)
                    listids = SvArrayCreate(NULL);
                SvArrayAddObject(listids, (SvGenericObject) listid);
                SVRELEASE(listid);
            }
        }
    }

    SvArray oldlistids = (SvArray) SvHashTableFind(self->channelToListids, (SvObject) channel);
    if (!SvObjectEquals((SvObject) listids, (SvObject) oldlistids))
        has_changed = true;

    SVTESTRELEASE(listids);


    if (has_changed) {
        QBNordigLCNChannelRemoved(self_, tvChannel);
        QBNordigLCNChannelFound(self_, tvChannel);
    } else {
        QBNordigLCNNotifyChannel(self, node);
    }
}

SvLocal void QBNordigLCNChannelRemoved(SvObject self_, SvTVChannel tvChannel)
{
    if (!SvObjectIsInstanceOf((SvObject) tvChannel, DVBEPGChannel_getType()))
        return;

    QBNordigLCN self = (QBNordigLCN) self_;

    DVBEPGChannel channel = (DVBEPGChannel) tvChannel;

    QBNordigLCNUnmarkReplacement(self, tvChannel);
    QBNordigLCNUnmarkLists(self, channel);

    SvValue id = SvDBObjectGetID((SvDBObject) tvChannel);
    QBNordigLCNNode node = (QBNordigLCNNode) SvHashTableFind(self->idToNode, (SvGenericObject) id);

    if (!node)
        return;

    SVRETAIN(node);

    //First just remove this channel
    SvBinaryTreeRemove(self->nodeByNumber, (SvGenericObject) node);
    SvHashTableRemove(self->idToNode, (SvGenericObject) id);
    SvBinaryTreeRemove(self->channelsWithLCN, (SvGenericObject) node);
    SvBinaryTreeRemove(self->duplicate, (SvGenericObject) node);

    bool require_primary_onid = self->primary_onid != -1 && self->use_primary_onid;
    bool has_primary_onid = self->primary_onid == channel->onid;

    if ((require_primary_onid && !has_primary_onid) || (node->lcn2 == -1 && node->lcn == -1)) {
        if (require_primary_onid && !has_primary_onid) {
            SvBinaryTreeRemove(self->nonPrimaryOnid, (SvGenericObject) node);
        } else {
            SvBinaryTreeRemove(self->channelsWithNoLCN, (SvGenericObject) node);
        }
    } else {
        SVAUTOINTVALUE(lcnVal, QBNordigLCNNodeGetLCN(node));
        SvBinaryTree duplicate = (SvBinaryTree) SvHashTableFind(self->nodesByLCN, (SvGenericObject) lcnVal);
        int dupIdx = SvBinaryTreeGetObjectIndex(duplicate, (SvGenericObject) node);
        int dupCnt = SvBinaryTreeGetNodesCount(duplicate);
        QBNordigLCNRemoveLCN(self, node);

        if (dupIdx == 0 && dupCnt > 1) {
            //If node wasnt a duplicate but there were some other duplicates
            //we need to take first of them and put it at the proper place
            //TODO: add a special case if the duplicate doesn't change position
            QBNordigLCNNode dupNode = (QBNordigLCNNode) SvBinaryTreeGetObjectAtIndex(duplicate, 0);
            SVRETAIN(dupNode);
            //first remove it
            SvBinaryTreeRemove(self->duplicate, (SvGenericObject) dupNode);
            SvBinaryTreeRemove(self->nodeByNumber, (SvGenericObject) dupNode);
            SvHashTableRemove(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) dupNode->channel));

            //then add it at another place
            SvHashTableInsert(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) dupNode->channel), (SvGenericObject) dupNode);
            dupNode->number = node->number;
            SvBinaryTreeInsert(self->nodeByNumber, (SvGenericObject) dupNode);
            SvBinaryTreeInsert(self->channelsWithLCN, (SvGenericObject) dupNode);
            QBNordigLCNNotifyChannel(self, dupNode);

            SVRELEASE(dupNode);
        }
    }

    QBNordigLCNNode prevNode = (QBNordigLCNNode) SvBinaryTreeGetObjectAtIndex(self->nodeByNumber, SvBinaryTreeGetNodesCount(self->channelsWithLCN) - 1);
    QBNordigLCNCorrectNumbering(self, prevNode, prevNode ? prevNode->number + 1 : 1);


    QBTunerMuxIdObj* muxid = QBTunerMuxIdObjCreate(&node->muxid);
    //Remove channel from the muxToChannels list
    SvHashTable channels = (SvHashTable) SvHashTableFind(self->muxToChannels, (SvGenericObject) muxid);
    if (channels) {
        SvHashTableRemove(channels, (SvGenericObject) channel);
        if (!SvHashTableCount(channels)) {
            SvHashTableRemove(self->muxToChannels, (SvGenericObject) muxid);
            channels = NULL;
        }
    }

    SvHashTable lcn2Channels = (SvHashTable) SvHashTableFind(self->lcn2MuxToChannels, (SvGenericObject) muxid);
    if (lcn2Channels) {
        SvHashTableRemove(lcn2Channels, (SvGenericObject) channel);
        //If that is the last lcn2 channel from this mux, then we need to notify all
        //other channels that they may have their lcns
        if (!SvHashTableCount(lcn2Channels)) {
            SvHashTableRemove(self->lcn2MuxToChannels, (SvGenericObject) muxid);
            if (channels) {
                SvArray array = SvArrayCreate(NULL);
                SvIterator it = SvHashTableKeysIterator(channels);
                SvArrayAddObjects(array, &it);
                it = SvArrayIterator(array);
                SvTVChannel itChannel;
                while ((itChannel = (SvTVChannel) SvIteratorGetNext(&it))) {
                    if (SvHashTableFind(self->idToNode, (SvGenericObject) SvDBObjectGetID((SvDBObject) itChannel)))
                        SvInvokeInterface(SvEPGChannelNumberMerger, self, mergeOnModified, itChannel);
                }
                SVRELEASE(array);
            }
        }
    }
    SVRELEASE(muxid);


    //Check if we were replacing a channel
    SvTVChannel replaced = (SvTVChannel) SvHashTableFind(self->replacementToChannel, (SvGenericObject) SvDBObjectGetID((SvDBObject)channel));
    if (replaced) {
        SvInvokeInterface(SvEPGChannelNumberMerger, self, mergeOnModified, replaced);
    }


    QBNordigLCNCorrectCountry(self);

    QBNordigLCNVerify(self);
    SVRELEASE(node);
}

SvLocal SvHashTable QBNordigLCNTakeResults(SvGenericObject self_)
{
    QBNordigLCN self = (QBNordigLCN) self_;

    SvHashTable ret = self->lcnChanges;
    self->lcnChanges = NULL;
    return ret;
}

void QBNordigLCNSetStartingNumberForLCNlessChannels(QBNordigLCN self, int value)
{
    self->startingNumberForLCNlessChannels = value;
}

SvLocal void QBNordigLCN__dtor__(void *self_)
{
    QBNordigLCN self = self_;

    SVRELEASE(self->nodesByLCN);

    SVRELEASE(self->lcn2MuxToChannels);
    SVRELEASE(self->muxToChannels);

    SVRELEASE(self->channelToReplacement);
    SVRELEASE(self->replacementToChannel);

    SVRELEASE(self->nodeByNumber);

    SVRELEASE(self->channelsWithNoLCN);

    SVRELEASE(self->idToNode);
    SVRELEASE(self->duplicate);

    SVRELEASE(self->nonPrimaryOnid);

    SVRELEASE(self->key);

    SVTESTRELEASE(self->filter);

    SVTESTRELEASE(self->currentCountry);

    SVRELEASE(self->listidToChannels);
    SVRELEASE(self->channelToListids);
    SVRELEASE(self->listidToCountry);

    SVRELEASE(self->channelsWithLCN);

    SVTESTRELEASE(self->lcnChanges);
}

SvType
QBNordigLCN_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNordigLCN__dtor__
    };
    static const struct SvEPGChannelNumberMerger_ lcnMerger = {
        .mergeOnFound = QBNordigLCNChannelFound,
        .mergeOnModified = QBNordigLCNChannelModified,
        .mergeOnRemoved = QBNordigLCNChannelRemoved,
        .takeResults = QBNordigLCNTakeResults,
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNordigLCN",
                            sizeof(struct QBNordigLCN_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvEPGChannelNumberMerger_getInterface(), &lcnMerger,
                NULL);
    }

    return type;
}

SvLocal void QBNordigLCNNode__dtor__(void *self_)
{
    QBNordigLCNNode self = self_;
    SVTESTRELEASE(self->channel);
}

SvLocal SvType
QBNordigLCNNode_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNordigLCNNode__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNordigLCNNode",
                            sizeof(struct QBNordigLCNNode_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

void QBNordigLCNInit(QBNordigLCN self, SvObject plugin, SvString filter, int primary_onid)
{
    if (plugin && !SvObjectIsInstanceOf(plugin, DVBEPGPlugin_getType())) {
        SvLogError("%s(): plugin is not of type DVBEPGPlugin", __func__);
        return;
    }

    self->nodesByLCN = SvHashTableCreate(123, NULL);
    self->lcn2MuxToChannels = SvHashTableCreate(123, NULL);

    self->muxToChannels = SvHashTableCreate(123, NULL);

    self->channelsWithNoLCN = SvBinaryTreeCreateWithCompareFn(QBNordigLCNCompareById, NULL, NULL);
    self->channelsWithLCN = SvBinaryTreeCreateWithCompareFn(QBNordigLCNCompareByNumber, NULL, NULL);

    self->replacementToChannel = SvHashTableCreate(123, NULL);
    self->channelToReplacement = SvHashTableCreate(123, NULL);

    self->idToNode = SvHashTableCreate(123, NULL);

    self->plugin = (DVBEPGPlugin) plugin;

    self->nodeByNumber = SvBinaryTreeCreateWithCompareFn(QBNordigLCNCompareByNumber, NULL, NULL);
    self->duplicate = SvBinaryTreeCreateWithCompareFn(QBNordigLCNCompareById, NULL, NULL);

    self->nonPrimaryOnid = SvBinaryTreeCreateWithCompareFn(QBNordigLCNCompareById, NULL, NULL);

    self->key = (QBNordigLCNNode) SvTypeAllocateInstance(QBNordigLCNNode_getType(), NULL);

    self->filter = SVTESTRETAIN(filter);

    self->primary_onid = primary_onid;

    self->listid = INT_MAX;
    self->listidToChannels = SvHashTableCreate(7, NULL);
    self->listidToCountry = SvHashTableCreate(7, NULL);
    self->channelToListids = SvHashTableCreate(123, NULL);

    self->startingNumberForLCNlessChannels = 1;
}


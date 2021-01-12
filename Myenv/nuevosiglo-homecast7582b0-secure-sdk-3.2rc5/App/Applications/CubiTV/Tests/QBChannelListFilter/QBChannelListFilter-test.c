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


#include <Services/QBChannelListFilter.h>
#include <SvEPGDataLayer/SvChannelMergerFilter.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <check.h>

extern void register_suite(Suite *s);

#define CHANNEL_LIST_PATH "/tmp/channelListFilter"

/**
 * Testing QBChannelListFilter create and destroy
 **/
START_TEST(QBChannelListFilter_create_and_destroy)
{
    QBChannelListFilter filter = QBChannelListFilterCreate();
    ck_assert_ptr_ne(filter, NULL);

    SVRELEASE(filter);
}
END_TEST

/**
 * Test QBChannelListFilter starting.
 **/
START_TEST(QBChannelListFilter_update_state)
{
    unsigned int preferredPluginID = 3;

    QBChannelListFilter filter = QBChannelListFilterCreate();
    ck_assert_ptr_ne(filter, NULL);

    bool enabled = SvInvokeInterface(SvChannelMergerFilter, filter, isEnabled);
    ck_assert(!enabled);

    enabled = SvInvokeInterface(SvChannelMergerFilter, filter, updateState, preferredPluginID);
    ck_assert(!enabled);

    QBChannelListFilterStart(filter, preferredPluginID, SVSTRING(CHANNEL_LIST_PATH));
    enabled = SvInvokeInterface(SvChannelMergerFilter, filter, updateState, preferredPluginID);
    ck_assert(!enabled);

    SVRELEASE(filter);
}
END_TEST

/**
 * Test if channels are allowed when plugin is disabled.
 **/
START_TEST(QBChannelListFilter_allow_when_disabled)
{
    bool allowed;
    unsigned int preferredPluginID = 3;
    SvValue channelID_A = SvValueCreateWithString(SVSTRING("id_A"), NULL);
    SvValue channelID_B = SvValueCreateWithString(SVSTRING("id_B"), NULL);

    QBChannelListFilter filter = QBChannelListFilterCreate();
    ck_assert_ptr_ne(filter, NULL);

    // filter isn't started, so all channels are allowed
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, channelID_A);
    ck_assert(allowed);

    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, channelID_B);
    ck_assert(allowed);

    QBChannelListFilterStart(filter, preferredPluginID, SVSTRING(CHANNEL_LIST_PATH));

    // filter have been started but not enabled, so all channels are allowed
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, channelID_A);
    ck_assert(allowed);

    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, channelID_B);
    ck_assert(allowed);

    SVRELEASE(channelID_A);
    SVRELEASE(channelID_B);
    SVRELEASE(filter);
}
END_TEST

/**
 * Test storing channels from preferred plugin.
 **/
START_TEST(QBChannelListFilter_adding_and_removing_channels)
{
    bool allowed;
    unsigned int preferredPluginID = 3;

    SvTVChannel channel_A = (SvTVChannel) SvTypeAllocateInstance(SvTVChannel_getType(), NULL);
    SvTVChannelInit(channel_A, SVSTRING("FakeChannel"), preferredPluginID, 1, NULL, SVSTRING("channelA"), NULL, NULL);

    SvTVChannel channel_B = (SvTVChannel) SvTypeAllocateInstance(SvTVChannel_getType(), NULL);
    SvTVChannelInit(channel_B, SVSTRING("FakeChannel"), preferredPluginID, 2, NULL, SVSTRING("channelB"), NULL, NULL);

    QBChannelListFilter filter = QBChannelListFilterCreate();
    ck_assert_ptr_ne(filter, NULL);

    // filter started and enabled
    QBChannelListFilterStart(filter, preferredPluginID, SVSTRING(CHANNEL_LIST_PATH));
    bool enabled = SvInvokeInterface(SvChannelMergerFilter, filter, updateState, preferredPluginID);
    ck_assert(!enabled);

    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(allowed);

    SvInvokeInterface(SvChannelMergerFilter, filter, channelFound, channel_A);
    enabled = SvInvokeInterface(SvChannelMergerFilter, filter, updateState, preferredPluginID);
    ck_assert(enabled);

    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(!allowed);

    SvInvokeInterface(SvChannelMergerFilter, filter, channelFound, channel_B);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(allowed);

    SvInvokeInterface(SvChannelMergerFilter, filter, channelLost, channel_A);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(!allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(allowed);

    SvInvokeInterface(SvChannelMergerFilter, filter, channelLost, channel_B);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(!allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(!allowed);

    enabled = SvInvokeInterface(SvChannelMergerFilter, filter, updateState, preferredPluginID);
    ck_assert(!enabled);

    SVRELEASE(filter);
    SVRELEASE(channel_A);
    SVRELEASE(channel_B);
}
END_TEST

/**
 * Test if filter keeps channels only with preferred plugin ID.
 **/
START_TEST(QBChannelListFilter_filtering_channels)
{
    bool allowed;
    unsigned int preferredPluginID = 3;
    unsigned int otherPluginID = 2;

    SvTVChannel channel_A = (SvTVChannel) SvTypeAllocateInstance(SvTVChannel_getType(), NULL);
    SvTVChannelInit(channel_A, SVSTRING("FakeChannel"), preferredPluginID, 1, NULL, SVSTRING("channelA"), NULL, NULL);

    SvTVChannel channel_B = (SvTVChannel) SvTypeAllocateInstance(SvTVChannel_getType(), NULL);
    SvTVChannelInit(channel_B, SVSTRING("FakeChannel"), otherPluginID, 2, NULL, SVSTRING("channelB"), NULL, NULL);

    QBChannelListFilter filter = QBChannelListFilterCreate();
    ck_assert_ptr_ne(filter, NULL);

    // filter started and enabled
    QBChannelListFilterStart(filter, preferredPluginID, SVSTRING(CHANNEL_LIST_PATH));
    SvInvokeInterface(SvChannelMergerFilter, filter, channelFound, channel_A);
    bool enabled = SvInvokeInterface(SvChannelMergerFilter, filter, updateState, preferredPluginID);
    ck_assert(enabled);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(!allowed);

    SvInvokeInterface(SvChannelMergerFilter, filter, channelFound, channel_B);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(!allowed);

    SvInvokeInterface(SvChannelMergerFilter, filter, channelLost, channel_A);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(!allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(!allowed);

    SvInvokeInterface(SvChannelMergerFilter, filter, channelLost, channel_B);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_A));
    ck_assert(!allowed);
    allowed = SvInvokeInterface(SvChannelMergerFilter, filter, isAllowed, SvTVChannelGetID(channel_B));
    ck_assert(!allowed);

    QBChannelListFilterStop(filter);
    SVRELEASE(filter);
    SVRELEASE(channel_A);
    SVRELEASE(channel_B);
}
END_TEST

static SvConstructor void init(void)
{
    Suite *s = suite_create("CubiTV");

    TCase *tc = tcase_create("QBChannelListFilter");
    suite_add_tcase(s, tc);

    tcase_add_test(tc, QBChannelListFilter_create_and_destroy);
    tcase_add_test(tc, QBChannelListFilter_allow_when_disabled);
    tcase_add_test(tc, QBChannelListFilter_update_state);
    tcase_add_test(tc, QBChannelListFilter_adding_and_removing_channels);
    tcase_add_test(tc, QBChannelListFilter_filtering_channels);

    register_suite(s);
}

/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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


#include "QBAccessManager_testCases.h"
#include "../common.h"
#include <Services/QBAccessController/PCAccessPlugin.h>
#include <Services/QBAccessController/Innov8onAccessPlugin.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvLog.h>
#include <fibers/c/fibers.h>
#include <main.h>


SvLocal AppGlobals appGlobals = NULL;

//test fixture setup function
SvLocal void setup(void)
{
    appGlobals = calloc(1, sizeof(struct AppGlobals_t));
    QBAccessManagerTest_initMinimalApplication(appGlobals);
}

//test fixture teardown function
SvLocal void teardown(void)
{
    QBAccessManagerTest_deinitMinimalApplication(appGlobals);
    free(appGlobals);
    appGlobals = NULL;
}

/**
 * tests that adding plugins to access manager works properly
 */
START_TEST(QBAccessManagerTestSimpleAddPlugins)
{
    ck_assert_msg(appGlobals->accessMgr, "qbAccessManager is NULL - cannot proceed");
    ck_assert_msg(appGlobals->middlewareManager, "qbMiddlewareManager is NULL - cannot proceed");

    //create and add new parental control plugin
    SvObject pluginPc = safeCreateTestObject(PCAccessPlugin_getType());
    __SAFE_CALL__(
        PCAccessPluginInit((PCAccessPlugin) pluginPc, 5, _error_);
        )
    __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginPc, SVSTRING("PC"), _error_);
        )

    //create and add new INNOV8ON plugin
    SvObject pluginInn = safeCreateTestObject(Innov8onAccessPlugin_getType());
    __SAFE_CALL__(
        Innov8onAccessPluginInit((Innov8onAccessPlugin) pluginInn, appGlobals->middlewareManager, 3, _error_);
        )
    __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginInn, SVSTRING("INNOV8ON"), _error_);
        )

    SVRELEASE(pluginPc);
    pluginPc = NULL;
    SVRELEASE(pluginInn);
    pluginInn = NULL;

    //run the scheduler so all the requests get processed
    SvSchedulerLoop(appGlobals->scheduler, false);
}
END_TEST

/**
 * tests that changing plugin for a domain works properly
 */
START_TEST(QBAccessManagerTestAddRemoveAddPlugins)
{
    ck_assert_msg(appGlobals->accessMgr, "qbAccessManager is NULL - cannot proceed");
    ck_assert_msg(appGlobals->middlewareManager, "qbMiddlewareManager is NULL - cannot proceed");

    //add PC plugin to manager
    SvObject pluginPc = safeCreateTestObject(PCAccessPlugin_getType());
    __SAFE_CALL__(
        PCAccessPluginInit((PCAccessPlugin) pluginPc, 5, _error_);
        )
    __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginPc, SVSTRING("PC"), _error_);
        )
    //add INNOV8ON plugin to manager
    SvObject pluginInn = safeCreateTestObject(Innov8onAccessPlugin_getType());
    __SAFE_CALL__(
        Innov8onAccessPluginInit((Innov8onAccessPlugin) pluginInn, appGlobals->middlewareManager, 3, _error_);
        )
    __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginInn, SVSTRING("INNOV8ON"), _error_);
        )
    //set INNOV8ON plugin as the plugin for dommain PC
    __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginInn, SVSTRING("PC"), _error_);
        )
    SvObject retPlugin = NULL;
    retPlugin = QBAccessManagerGetDomainPlugin(appGlobals->accessMgr, SVSTRING("PC"));

    SVRETAIN(retPlugin);
    //check that plugin for domain PC is now the same as plugin for domain "INNOV8ON"
    ck_assert_msg(retPlugin == pluginInn, "Access Manager for domain PC should be the same ass for domain INNOV8ON as it was changed");

    SVRELEASE(retPlugin);
    retPlugin = NULL;
    SVRELEASE(pluginPc);
    pluginPc = NULL;
    SVRELEASE(pluginInn);
    pluginInn = NULL;

    SvSchedulerLoop(appGlobals->scheduler, false);
}
END_TEST

/**
 * tests that default plugin can be switched to a different plugin and that it is returned properly from access manager
 */
START_TEST(QBAccessManagerTestChangeDefaultPlugin)
{
    ck_assert_msg(appGlobals->accessMgr, "qbAccessManager is NULL - cannot proceed");

    //create a plugin
    SvObject pluginPc = safeCreateTestObject(PCAccessPlugin_getType());
    __SAFE_CALL__(
        PCAccessPluginInit((PCAccessPlugin) pluginPc, 5, _error_);
        )
    //overwrite default plugin
    __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginPc, NULL, _error_);
        )

    //get default plugin in two different ways
    SvObject defPlugin = QBAccessManagerGetDomainPlugin(appGlobals->accessMgr, SVSTRING("DEFAULT"));
    SVRETAIN(defPlugin);
    SvObject defPlugin2 = QBAccessManagerGetDomainPlugin(appGlobals->accessMgr, NULL);
    SVRETAIN(defPlugin2);

    //assert that default plugin is always returned properly
    ck_assert_msg(defPlugin != NULL, "Default Access Manager plugin cannot be NULL");
    ck_assert_msg(defPlugin2 != NULL, "Default Access Manager plugin cannot be NULL");
    ck_assert_msg(defPlugin == defPlugin2, "Default Access Manager plugin not returned properly");

    SVRELEASE(defPlugin);
    defPlugin = NULL;
    SVRELEASE(pluginPc);
    pluginPc = NULL;
    SVRELEASE(defPlugin2);
    defPlugin2 = NULL;

    SvSchedulerLoop(appGlobals->scheduler, false);
}
END_TEST

TCase* QBAccessManagetTest_addPlugins_testCase(void)
{
    TCase* tcase = tcase_create("addPlugins");

    //setup and teardown will be called before and after each test
    tcase_add_checked_fixture(tcase, setup, teardown);
    tcase_add_loop_test(tcase, QBAccessManagerTestSimpleAddPlugins, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestAddRemoveAddPlugins, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestChangeDefaultPlugin, 0, 1);
    tcase_set_timeout(tcase, 15);

    return tcase;
}

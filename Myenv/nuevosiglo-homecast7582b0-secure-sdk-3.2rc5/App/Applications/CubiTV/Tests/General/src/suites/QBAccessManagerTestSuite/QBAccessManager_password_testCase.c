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
#include "stubs/QBAccessManagerListenerStub.h"
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

    ck_assert_msg(appGlobals->accessMgr, "qbAccessManager is NULL - cannot proceed");
    ck_assert_msg(appGlobals->middlewareManager, "qbMiddlewareManager is NULL - cannot proceed");

    SvObject pluginPc = safeCreateTestObject(PCAccessPlugin_getType());
    //add the PC and INNOV8ON plugins to access manager
    __SAFE_CALL__(
        PCAccessPluginInit((PCAccessPlugin) pluginPc, 5, _error_);
        ) __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginPc, SVSTRING("PC"), _error_);
        )

    SvObject pluginInn = safeCreateTestObject(Innov8onAccessPlugin_getType());
    __SAFE_CALL__(
        Innov8onAccessPluginInit((Innov8onAccessPlugin) pluginInn, appGlobals->middlewareManager, 3, _error_);
        ) __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginInn, SVSTRING("INNOV8ON"), _error_);
        )

    SVRELEASE(pluginPc);
    pluginPc = NULL;
    SVRELEASE(pluginInn);
    pluginInn = NULL;
}

//test fixture teardown function
SvLocal void teardown(void)
{
    QBAccessManagerTest_deinitMinimalApplication(appGlobals);
    free(appGlobals);
    appGlobals = NULL;
}

/**
 * tests if password changing works properly
 */
START_TEST(QBAccessManagerTestChangePassword)
{
    //request password change
    QBPasswordStatus status = QBPasswordStatus_unknown;
    __SAFE_CALL__(
        status = QBAccessManagerChangePassword(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), SVSTRING("2222"), _error_);
        )
    //check that password change is in progress
    SvLogNotice("Password change status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBPasswordStatus_inProgress, "Password change status should be 'In Progress (4)' before running scheduler loop");

    //spin the scheduler to process password change request
    SvSchedulerLoop(appGlobals->scheduler, false);

    //check that password was changed
    __SAFE_CALL__(
        status = QBAccessManagerCheckPasswordChange(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    SvLogNotice("Password change status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBPasswordStatus_OK, "Password change status should be 'OK (1)' after running scheduler loop");
}
END_TEST

/**
 * tests if password reset works properly
 */
START_TEST(QBAccessManagerTestResetPassword)
{
    //request password reset
    QBPasswordStatus status = QBPasswordStatus_unknown;
    __SAFE_CALL__(
        status = QBAccessManagerResetPassword(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("2222"), _error_);
        )
    //check that password reset fails as it should
    SvLogNotice("Password reset status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBPasswordStatus_failed, "Password change status should be 'Failed' for dommain INNOV8ON");
}
END_TEST

TCase* QBAccessManagetTest_password_testCase(void)
{
    TCase* tcase = tcase_create("password");

    tcase_add_checked_fixture(tcase, setup, teardown);
    tcase_add_loop_test(tcase, QBAccessManagerTestChangePassword, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestResetPassword, 0, 1);
    tcase_set_timeout(tcase, 15);

    return tcase;
}

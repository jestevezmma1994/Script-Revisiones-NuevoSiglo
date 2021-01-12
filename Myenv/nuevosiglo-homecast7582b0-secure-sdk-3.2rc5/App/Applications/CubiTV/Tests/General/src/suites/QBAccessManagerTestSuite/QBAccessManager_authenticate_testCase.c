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
    //add the PC and INNOV8ON plugins to access manager
    SvObject pluginPc = safeCreateTestObject(PCAccessPlugin_getType());
    __SAFE_CALL__(
        PCAccessPluginInit((PCAccessPlugin) pluginPc, 5, _error_);
        )
    __SAFE_CALL__(
        QBAccessManagerSetPluginByDomain(appGlobals->accessMgr, pluginPc, SVSTRING("PC"), _error_);
        )

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
}

//test fixture teardown function
SvLocal void teardown(void)
{
    QBAccessManagerTest_deinitMinimalApplication(appGlobals);
    free(appGlobals);
    appGlobals = NULL;
}

/**
 * tests that authentication with access manager works properly
 */
START_TEST(QBAccessManagerTestSimpleAuthenticate)
{
    SvLogNotice("STARTING TEST: QBAccessManagerTestSimpleAuthenticate");

    QBAuthStatus status = QBAuthStatus_unknown;
    __SAFE_CALL__(
        status = QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), NULL, _error_);
        )
    //check that, just after authentication starts, its status is "in progress".
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBAuthStatus_inProgress, "Authentication status should be 'In Progress (2)' before running scheduler loop");

    //process the authentication request
    SvSchedulerLoop(appGlobals->scheduler, false);

    __SAFE_CALL__(
        status = QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    //check if in the and authentication succeeds
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBAuthStatus_OK, "Authentication status should be 'OK (0)' after running scheduler loop");
}
END_TEST

/**
 * tests that listener get called when authentication changes
 */
START_TEST(QBAccessManagerTestListenerCalledOnAuthenticate)
{
    SvLogNotice("STARTING TEST: QBAccessManagerTestListenerCalledOnAuthenticate");

    //create and register listener in access manager
    SvObject authListener = safeCreateTestObject(QBAccessManagerListenerStub_getType());
    __SAFE_CALL__(
        QBAccessManagerAddListener(appGlobals->accessMgr, authListener, _error_);
        )

    //request authentication - (authentication process calls listener twice - at the beginning and end of authentication)
    __SAFE_CALL__(
        QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), NULL,  _error_);
        )

    //check that a listener was called at the beginning of authentication process
    int listenerCallTimes = QBAccessManagerGetCallTimes((QBAccessManagerListenerStub) authListener);
    ck_assert_msg(listenerCallTimes == 1, "Listener should be called once by this time");

    //process authentication request
    SvSchedulerLoop(appGlobals->scheduler, false);
    //force refresh of quthentication status (calls listeners on authentication process end)
    __SAFE_CALL__(
        QBAccessManagerCheckAuthentication(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    //check that listener was called again
    listenerCallTimes = QBAccessManagerGetCallTimes((QBAccessManagerListenerStub) authListener);
    ck_assert_msg(listenerCallTimes == 2, "Listener should be called twice by this time");

    SVRELEASE(authListener);
    authListener = NULL;
}
END_TEST

/**
 * tests if a listener will not be called after it is removed from listeners list
 */
START_TEST(QBAccessManagerTestListenerNotCalledOnAuthenticateWhenRemoved)
{
    SvLogNotice("STARTING TEST: QBAccessManagerTestListenerNotCalledOnAuthenticateWhenRemoved");

    //create and register listener in access manager
    SvObject authListener = safeCreateTestObject(QBAccessManagerListenerStub_getType());
    __SAFE_CALL__(
        QBAccessManagerAddListener(appGlobals->accessMgr, authListener, _error_);
        )

    //remove listener from manager
    __SAFE_CALL__(
        QBAccessManagerRemoveListener(appGlobals->accessMgr, authListener, _error_);
        )

    //request authentication - (authentication process calls listeners twice - at the beginning and end of authentication)
    __SAFE_CALL__(
        QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), NULL, _error_);
        )
    //check that a listener was not called (it was removed from listeners list)
    int listenerCallTimes = QBAccessManagerGetCallTimes((QBAccessManagerListenerStub) authListener);
    ck_assert_msg(listenerCallTimes == 0, "Listener should NOT be called at all in this test");

    //process authentication events
    SvSchedulerLoop(appGlobals->scheduler, false);
    __SAFE_CALL__(
        QBAccessManagerCheckAuthentication(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )

    //check that a listener was not called (it was removed from listeners list)
    listenerCallTimes = QBAccessManagerGetCallTimes((QBAccessManagerListenerStub) authListener);
    ck_assert_msg(listenerCallTimes == 0, "Listener should NOT be called at all in this test");

    SVRELEASE(authListener);
    authListener = NULL;
}
END_TEST

/**
 * tests if authentication timeout works properly
 */
START_TEST(QBAccessManagerTestAuthenticationTimer)
{
    SvLogNotice("STARTING TEST: QBAccessManagerTestAuthenticationTimer");

    //request authentication
    QBAuthStatus status = QBAuthStatus_unknown;
    __SAFE_CALL__(
        status = QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), NULL, _error_);
        )
    //process authentication
    SvSchedulerLoop(appGlobals->scheduler, false);

    //make sure that authentication succeed
    __SAFE_CALL__(
        status = QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBAuthStatus_OK, "Authentication status should be 'OK (0)' after running scheduler loop");

    //set the timeout of authentication (one second)
    __SAFE_CALL__(
        QBAccessManagerSetAuthenticationValidityTime(appGlobals->accessMgr, SVSTRING("INNOV8ON"), 2, _error_);
        )
    //wait until authentication timeout expires
    SvSchedulerLoop(appGlobals->scheduler, false);

    //make sure that authentication is not valid
    __SAFE_CALL__(
        status = QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status != QBAuthStatus_OK, "Authentication status should not be 'OK (0)' after running scheduler loop for the second time (validity time passed)");
}
END_TEST

/**
 * tests if authentication invalidation really invalidates authentication
 */
START_TEST(QBAccessManagerTestAuthenticationInvalidation)
{
    SvLogNotice("STARTING TEST: QBAccessManagerTestAuthenticationInvalidation");

    //request authentication
    QBAuthStatus status = QBAuthStatus_unknown;
    __SAFE_CALL__(
        status = QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), NULL, _error_);
        )
    //process authentication request
    SvSchedulerLoop(appGlobals->scheduler, false);

    //make sure that authentication was successfull
    __SAFE_CALL__(
        status = QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBAuthStatus_OK, "Authentication status should be 'OK (0)' after running scheduler loop");

    //invalidate authentication
    __SAFE_CALL__(
        QBAccessManagerInvalidateAuthentication(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )

    //make sure that authentication is not valid anymore
    __SAFE_CALL__(
        status = QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status != QBAuthStatus_OK, "Authentication status should should not be 'OK (0)' after invalidation");
}
END_TEST

/**
 * tests if revoking authentication cancels authentication process
 */
START_TEST(QBAccessManagerTestRevokeAuthentication)
{
    SvLogNotice("STARTING TEST: QBAccessManagerTestRevokeAuthentication");

    QBAuthStatus status = QBAuthStatus_unknown;
    //request authentiaction
    __SAFE_CALL__(
        status = QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), NULL, _error_);
        )
    //make sure authentication is in progress
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBAuthStatus_inProgress, "Authentication status should be 'In Progress (2)' before running scheduler loop, and before revoking");

    //revoke authentication process
    __SAFE_CALL__(
        QBAccessManagerRevokeAuthenticationByDomain(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    //check that authentication is no longer in progress
    __SAFE_CALL__(
        status = QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBAuthStatus_unknown, "Authentication status should be 'Unknown (255)' before running scheduler loop, and after revoking");

    //request another authentication
    __SAFE_CALL__(
        status = QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), NULL, _error_);
        )
    //process authentiacation request
    SvSchedulerLoop(appGlobals->scheduler, false);

    //make sure authentication succeeded
    __SAFE_CALL__(
        status = QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBAuthStatus_OK, "Authentication status should be 'OK (0)' after running scheduler loop");

    //try to revoke authentication
    __SAFE_CALL__(
        QBAccessManagerRevokeAuthenticationByDomain(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )

    //make sure that revoking had no effect (it happened after authentication succeeded)
    __SAFE_CALL__(
        status = QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    SvLogNotice("Authentication status for domain INNOV8ON %d", status);
    ck_assert_msg(status == QBAuthStatus_OK, "Authentication status should be 'OK (0)' after revoking when authentication managed to pass before revoke"); //TODO check if it should really be unknown - authentication passed, and then revoked (difference with invalidate)
}
END_TEST

/**
 * tests if checking access to a domain works properly
 */
START_TEST(QBAccessManagerTestCheckAccess)
{
    SvLogNotice("STARTING TEST: QBAccessManagerTestCheckAccess");

    //checking if access to specific domains without authentication is correct
    QBAccessCode accesCode = QBAccessManagerCheckAccess(appGlobals->accessMgr, SVSTRING("INNOV8ON"));
    SvLogNotice("Access code for domain INNOV8ON %d", accesCode);
    ck_assert_msg(accesCode == QBAccess_confirmationRequired, "INNOV8ON - access confirmation should be required before authentication");

    accesCode = QBAccessManagerCheckAccess(appGlobals->accessMgr, SVSTRING("PC"));
    SvLogNotice("Access code for domain PC %d", accesCode);
    ck_assert_msg(accesCode == QBAccess_confirmationRequired, "PC - access confirmation should be required before authentication");

    accesCode = QBAccessManagerCheckAccess(appGlobals->accessMgr, NULL);
    SvLogNotice("Access code for domain DEFAULT %d", accesCode);
    ck_assert_msg(accesCode == QBAccess_granted, "DEFAULT - access should be granted as we didn't set any blocked domains in QBStaticAccessPluginInit");

    //domains authentication
    __SAFE_CALL__(
        QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("1111"), NULL, _error_);
        )

    __SAFE_CALL__(
        QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("PC"), SVSTRING("1111"), NULL, _error_);
        )
    //authentication events processing
    SvSchedulerLoop(appGlobals->scheduler, false);

    //checking if access to specific domains after authentication is correct
    accesCode = QBAccessManagerCheckAccess(appGlobals->accessMgr, SVSTRING("INNOV8ON"));
    SvLogNotice("Access code for domain INNOV8ON %d", accesCode);
    ck_assert_msg(accesCode == QBAccess_confirmationRequired, "INNOV8ON - access confirmation should be required after authentication");

    accesCode = QBAccessManagerCheckAccess(appGlobals->accessMgr, SVSTRING("PC"));
    SvLogNotice("Access code for domain PC %d", accesCode);
    ck_assert_msg(accesCode == QBAccess_granted, "PC - access should be granted after authentication");
}
END_TEST

/**
 * tests gewttting authorization parameters for a domain
 */
START_TEST(QBAccessManagerTestGetAuthParams)
{
    SvLogNotice("STARTING TEST: QBAccessManagerTestGetAuthParams");

    QBAuthParams params;
    //getting domains authentication parameters
    __SAFE_CALL__(
        QBAccessManagerGetAuthParamsByDomain(appGlobals->accessMgr, &params, SVSTRING("INNOV8ON"), _error_);
        )
}
END_TEST


TCase* QBAccessManagetTest_authenticate_testCase(void)
{
    TCase* tcase = tcase_create("authenticate");

    //setup and teardown will be called before and after each test
    tcase_add_checked_fixture(tcase, setup, teardown);

    //add all tests to this test case
    tcase_add_loop_test(tcase, QBAccessManagerTestSimpleAuthenticate, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestListenerCalledOnAuthenticate, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestListenerNotCalledOnAuthenticateWhenRemoved, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestAuthenticationTimer, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestAuthenticationInvalidation, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestRevokeAuthentication, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestCheckAccess, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestGetAuthParams, 0, 1);
    tcase_set_timeout(tcase, 15);

    return tcase;
}

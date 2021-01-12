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
 * tests if setting pin attempts as unlimited works properly
 */
START_TEST(QBAccessManagerTestPinAttemptsUnlimiting)
{
    //check how many pin attempts remain for dommain PC, and makes sure attempts are not unlimited by default
    int pcPinsLeft = QBAccessManagerGetPinAttemptsLeft(appGlobals->accessMgr, SVSTRING("PC"));
    ck_assert_msg(pcPinsLeft != -1, "Pin attempts should NOT be unlimited before call to QBAccessManagerSetPinAttemptsAsUnlimited with proper domain name");

    //sets attempts as unlimited
    QBAccessManagerSetPinAttemptsAsUnlimited(appGlobals->accessMgr, SVSTRING("PC"));

    //chceks that pins attempts are unlimited
    pcPinsLeft = QBAccessManagerGetPinAttemptsLeft(appGlobals->accessMgr, SVSTRING("PC"));
    ck_assert_msg(pcPinsLeft == -1, "Pin attempts should be unlimited after call to QBAccessManagerSetPinAttemptsAsUnlimited");
}
END_TEST

SvLocal SvFiber blockPinsFiber = NULL;
SvLocal int lastCheckPinAttemptsLeft = -1;

/**
 * local fiber for drying out pin attempts
 */
SvLocal void QBAccessManagerTestBlockPinsFiber(void* fiber)
{
    SvFiberDeactivate(blockPinsFiber);

    //make the pin attempts be checked at all
    __SAFE_CALL__(
        QBAccessManagerCheckAuthentication(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )

    //assert that pin attempts only drop (not necessarily in every fiber call)
    int pinsLeft = QBAccessManagerGetPinAttemptsLeft(appGlobals->accessMgr, SVSTRING("INNOV8ON"));
    ck_assert_msg(pinsLeft == lastCheckPinAttemptsLeft || pinsLeft == lastCheckPinAttemptsLeft - 1,
                  "pin attempts should slowly decrease to 0 in steps by 1");

    //if pin attempts dropped from last fiber call attempt another wrong authentication
    if (pinsLeft < lastCheckPinAttemptsLeft) {
        __SAFE_CALL__(
            QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("9999"), NULL, _error_);
            )
    }
    lastCheckPinAttemptsLeft = pinsLeft;
    //call the fiber again if we have some attempts left
    if (lastCheckPinAttemptsLeft > 0) {
        SvFiberActivate(blockPinsFiber);
    } else {
        //if no attempts left - assert that there are exactly zero attempts left
        ck_assert_msg(lastCheckPinAttemptsLeft == 0, "pin attempts should reach 0 eventually");

        //unjam scheduler and finish up this fiber
        int blockPeriodLeft = QBAccessManagerGetBlockPeriodLeft(appGlobals->accessMgr);
        SvLogNotice("Block period left - before reset: %d", blockPeriodLeft);
        QBAccessManagerResetPinAttemptsLeft(appGlobals->accessMgr);
        blockPeriodLeft = QBAccessManagerGetBlockPeriodLeft(appGlobals->accessMgr);
        SvLogNotice("Block period left - after reset: %d", blockPeriodLeft);
        SvLogNotice("QBAccessManagerTestBlockPinsFiber - unjamming");
        SvFiberDestroy(blockPinsFiber);
    }
}

/**
 * tests if PIN blocking works properly
 */
START_TEST(QBAccessManagerTestBlockingPin)
{
    //check how many pin attempts remain for INNOV8ON and make sure it is not -1
    int innPinsLeft = QBAccessManagerGetPinAttemptsLeft(appGlobals->accessMgr, SVSTRING("INNOV8ON"));
    ck_assert_msg(innPinsLeft != -1, "Pin attempts should NOT be unlimited before call to QBAccessManagerSetPinAttemptsAsUnlimited with proper domain name");
    SvLogNotice("INNOV8ON pin attempts left: %d", innPinsLeft);

    //we start with +1 so the fiber starts from wrong authentication
    lastCheckPinAttemptsLeft = innPinsLeft + 1;

    //create fiber that will dry out pin attempts
    blockPinsFiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBAccessManagerTestBlockPinsFiber", QBAccessManagerTestBlockPinsFiber, NULL);
    SvFiberActivate(blockPinsFiber);
    SvSchedulerLoop(appGlobals->scheduler, false);
}
END_TEST

static SvFiber unjamFiber = NULL;

//fiber that terminates waiting for 15 minutes (pin block period)
SvLocal void QBAccessManagerTestUnjamFiber(void* fiber)
{
    SvFiberDeactivate(unjamFiber);
    QBAccessManagerResetPinAttemptsLeft(appGlobals->accessMgr);
    int blockPeriodLeft = QBAccessManagerGetBlockPeriodLeft(appGlobals->accessMgr);
    SvLogNotice("Block period left: %d", blockPeriodLeft);
    SvLogNotice("QBAccessManagerTest_unjamFiber - unjamming");
    SvFiberDestroy(unjamFiber);
}

/**
 * tests if blocking happens
 * WARNING! test is long - about 15 seconds - env CK_DEFAULT_TIMEOUT should be set to more than 15 (default is 4)
 */
START_TEST(QBAccessManagerTestBlockingHappens)
{
    //check how many pin attempts remain for INNOV8ON and make sure it is not -1
    int innPinsLeft = QBAccessManagerGetPinAttemptsLeft(appGlobals->accessMgr, SVSTRING("INNOV8ON"));
    ck_assert_msg(innPinsLeft != -1, "Pin attempts should NOT be unlimited before call to QBAccessManagerSetPinAttemptsAsUnlimited with proper domain name");
    SvLogNotice("INNOV8ON pin attempts left: %d", innPinsLeft);

    //request authentication with a wrong PIN
    __SAFE_CALL__(
        QBAccessManagerAuthenticate(appGlobals->accessMgr, SVSTRING("INNOV8ON"), SVSTRING("9999"), NULL, _error_);
        )
    //spin the scheduler and make sure authentication attempts are handled
    SvSchedulerLoop(appGlobals->scheduler, false);
    __SAFE_CALL__(
        QBAccessManagerCheckAuthentication(appGlobals->accessMgr, SVSTRING("INNOV8ON"), _error_);
        )
    //setup fiber that will cause the test to stop before 15 minutes pass
    unjamFiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBAccessManagerTestUnjamFiber", QBAccessManagerTestUnjamFiber, NULL);
    SvFiberTimer unjamTimer = SvFiberTimerCreate(unjamFiber);
    SvFiberTimerActivateAfter(unjamTimer, SvTimeConstruct(16, 0));

    SvSchedulerLoop(appGlobals->scheduler, false);
}
END_TEST


TCase* QBAccessManagetTest_blocking_testCase(void)
{
    TCase* tcase = tcase_create("blocking");

    //setup and teardown will be called before and after each test
    tcase_add_checked_fixture(tcase, setup, teardown);

    //add all tests to this test case
    tcase_add_loop_test(tcase, QBAccessManagerTestPinAttemptsUnlimiting, 0, 1);
    tcase_add_loop_test(tcase, QBAccessManagerTestBlockingPin, 0, 1);
    if (QBTestsLongTestsEnabled()) {
        tcase_add_loop_test(tcase, QBAccessManagerTestBlockingHappens, 0, 1);
    }
    tcase_set_timeout(tcase, 15);

    return tcase;
}

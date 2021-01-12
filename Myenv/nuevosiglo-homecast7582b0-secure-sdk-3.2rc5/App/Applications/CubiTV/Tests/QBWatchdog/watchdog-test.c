/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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
** (2) distributiot third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include <check.h>
#include <string.h> //strcmp
#include <stdbool.h>
#include <Services/core/watchdog.h>
#include <SvFoundation/SvObject.h>
#include <QBPlatformHAL/QBPlatformWatchdog.h>
#include <fibers/c/fibers.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>



// ------------------------- QBWatchdog tests ------------------------

extern void register_suite(Suite *s);
static SvScheduler scheduler = NULL;

// check if function returns proper error
SvLocal bool SvErrorInfoDomainAndCodeIsValid(SvErrorInfo error, const char *domain, int code)
{
    if (error == NULL) {
        return true;
    }
    return (!strcmp(SvErrorInfoGetDomain(error), domain) && SvErrorInfoGetCode(error) == code);
}

// destroy SvErrorInfo object with displaying error log
SvLocal void SvErrorInfoDisplayDestroy(SvErrorInfo error)
{
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

// testing QBWatchdog create and destroy
START_TEST(QBWatchdog_create_destroy)
{
    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();
    SvErrorInfo error = NULL;
    QBWatchdog wdg;

    // try create QBWatchdog, should return without error
    wdg = QBWatchdogCreate(&error);
    ck_assert(error == NULL);
    ck_assert(wdg);

    // destroy QBWatchdog
    SVRELEASE(wdg);

    SvSchedulerLoop(scheduler, false);
    SvSchedulerCleanup();
    scheduler = NULL;
}
END_TEST

// testing start and stop function with different arguments
START_TEST(QBWatchdog_start_stop)
{
    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();
    SvErrorInfo error = NULL;
    QBWatchdog wdg;

    // try create QBWatchdog, should return without error
    wdg = QBWatchdogCreate(&error);
    ck_assert(error == NULL);

    // try start QBWatchdog with all argument invalid, should return SvCoreError_invalidArgument
    QBWatchdogStart(NULL, NULL, 301, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try start QBWatchdog with only first argument valid, should return SvCoreError_invalidArgument
    QBWatchdogStart(wdg, NULL, 301, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try start QBWatchdog with too large timeout argument, should return SvCoreError_invalidArgument
    QBWatchdogStart(wdg, scheduler, 301, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try start QBWatchdog with all argument valid, should return without error
    QBWatchdogStart(wdg, scheduler, 10, &error);
    ck_assert(error == NULL);

    // try start started QBWatchdog with all argument valid, should display warning log and timeout is set
    QBWatchdogStart(wdg, scheduler, 20, &error);
    ck_assert(error == NULL);

    // try start QBWatchdog with NULL self, should return SvCoreError_invalidArgument
    QBWatchdogStart(NULL, scheduler, 10, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try start QBWatchdog with negative timeout, should return SvCoreError_invalidArgument
    QBWatchdogStart(wdg, scheduler, -1, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try stop QBWatchdog with NULL self, should return SvCoreError_invalidArgument
    QBWatchdogStop(NULL, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try stop QBWatchdog, should return without error
    QBWatchdogStop(wdg, &error);
    ck_assert(error == NULL);

    // destroy QBWatchdog
    SVRELEASE(wdg);

    SvSchedulerLoop(scheduler, false);
    SvSchedulerCleanup();
    scheduler = NULL;
}
END_TEST

// testing set timeout function
START_TEST(QBWatchdog_set_timeout)
{
    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();
    SvErrorInfo error = NULL;
    QBWatchdog wdg;

    // try create QBWatchdog, should return without error
    wdg = QBWatchdogCreate(&error);
    ck_assert(error == NULL);

    // try start QBWatchdog with all argument valid, should return without error
    QBWatchdogStart(wdg, scheduler, 1, &error);
    ck_assert(error == NULL);

    // try set timeout to QBWatchdog with NULL self, should return SvCoreError_invalidArgument
    QBWatchdogSetTimeout(NULL, 10, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try set timeout to QBWatchdog with NULL self and negative timeout, should return SvCoreError_invalidArgument
    QBWatchdogSetTimeout(NULL, -1, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try set timeout to QBWatchdog with all argument valid, should return without error
    QBWatchdogSetTimeout(wdg, 10, &error);
    ck_assert(error == NULL);

    // try set timeout to QBWatchdog with all argument valid, should return without error
    QBWatchdogSetTimeout(wdg, 20, &error);
    ck_assert(error == NULL);

    // try set timeout to QBWatchdog with nevative timeout, should return SvCoreError_invalidArgument
    QBWatchdogSetTimeout(wdg, -1, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try set timeout to QBWatchdog with timeout equal to 0, should stop QBWatchdog without error
    QBWatchdogSetTimeout(wdg, 0, &error);
    ck_assert(error == NULL);

    // try set timeout to QBWatchdog with nevative timeout, should return SvCoreError_invalidArgument
    QBWatchdogSetTimeout(wdg, -10, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try set timeout to QBWatchdog with nevative timeout, should return SvCoreError_invalidArgument
    QBWatchdogSetTimeout(wdg, -1024, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try stop QBWatchdog, should return without error
    QBWatchdogStop(wdg, &error);
    ck_assert(error == NULL);

    // destroy QBWatchdog
    SVRELEASE(wdg);

    SvSchedulerLoop(scheduler, false);
    SvSchedulerCleanup();
    scheduler = NULL;
 }
 END_TEST

 // testing QBWatchdog feed
 START_TEST(QBWatchdog_continue)
 {
    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();
    SvErrorInfo error = NULL;
    QBWatchdog wdg;

    // try create QBWatchdog, should return without error
    wdg = QBWatchdogCreate(&error);
    ck_assert(error == NULL);

    // try start QBWatchdog, should return without error
    QBWatchdogStart(wdg, scheduler, 1, &error);
    ck_assert(error == NULL);

    // try feed QBWatchdog with NULL self, should return SvCoreError_invalidArgument
    QBWatchdogContinue(NULL, &error);
    ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDisplayDestroy(error);

    // try feed QBWatchdog, should return without error
    QBWatchdogContinue(wdg, &error);
    ck_assert(error == NULL);

    // try stop QBWatchdog, should return without error
    QBWatchdogStop(wdg, &error);
    ck_assert(error == NULL);

    // destroy QBWatchdog
    SVRELEASE(wdg);

    SvSchedulerLoop(scheduler, false);
    SvSchedulerCleanup();
    scheduler = NULL;
 }
 END_TEST

 // testing reboot after default timeout
 START_TEST(QBWatchdog_reboot)
 {
     SvSchedulerCreateMain();
     scheduler = SvSchedulerGet();
     SvErrorInfo error = NULL;
     QBWatchdog wdg;

     // try create QBWatchdog, should return without error
     wdg = QBWatchdogCreate(&error);
     ck_assert(error == NULL);

     // try start QBWatchdog, should return without error
     QBWatchdogStart(wdg, scheduler, 1, &error);
     ck_assert(error == NULL);

     // try reboot QBWatchdog with default timeout with NULL self, should return SvCoreError_invalidArgument
     QBWatchdogReboot(NULL, &error);
     ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
     SvErrorInfoDisplayDestroy(error);

     // try reboot QBWatchdog with defult timeout, should return without error
     QBWatchdogReboot(wdg, &error);
     ck_assert(error == NULL);

     // try stop QBWatchdog, should return without error
     QBWatchdogStop(wdg, &error);
     ck_assert(error == NULL);

     // destroy QBWatchdog
     SVRELEASE(wdg);

     SvSchedulerLoop(scheduler, false);
     SvSchedulerCleanup();
     scheduler = NULL;
 }
 END_TEST

 // testing reboot after given timeout
 START_TEST(QBWatchdog_reboot_after_timeout)
 {
     SvSchedulerCreateMain();
     scheduler = SvSchedulerGet();
     QBWatchdog wdg;
     SvErrorInfo error = NULL;

     // try create QBWatchdog, should return without error
     wdg = QBWatchdogCreate(&error);
     ck_assert(error == NULL);

     // try start QBWatchdog, should return without error
     QBWatchdogStart(wdg, scheduler, 1, &error);
     ck_assert(error == NULL);

     // try reboot QBWatchdog with NULL self, should return SvCoreError_invalidArgument
     QBWatchdogRebootAfterTimeout(NULL, 0, &error);
     ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
     SvErrorInfoDisplayDestroy(error);

     // try reboot QBWatchdog with given timeout, should return without error
     QBWatchdogRebootAfterTimeout(wdg, 4, &error);
     ck_assert(error == NULL);

     // try reboot QBWatchdog with too large timeout, should return SvCoreError_invalidArgument
     QBWatchdogRebootAfterTimeout(wdg, 301, &error);
     ck_assert(SvErrorInfoDomainAndCodeIsValid(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
     SvErrorInfoDisplayDestroy(error);

     // try stop QBWatchdog, should return without error
     QBWatchdogStop(wdg, &error);
     ck_assert(error == NULL);

     // destroy QBWatchdog
     SVRELEASE(wdg);

     SvSchedulerLoop(scheduler, false);
     SvSchedulerCleanup();
     scheduler = NULL;
 }
 END_TEST

static SvConstructor void init(void)
{
    Suite *s = suite_create("CubiTV");
    TCase *tc = tcase_create("QBWatchdog");
    suite_add_tcase(s, tc);

    tcase_add_test(tc, QBWatchdog_create_destroy);
    tcase_add_test(tc, QBWatchdog_start_stop);
    tcase_add_test(tc, QBWatchdog_set_timeout);
    tcase_add_test(tc, QBWatchdog_continue);
    tcase_add_test(tc, QBWatchdog_reboot);
    tcase_add_test(tc, QBWatchdog_reboot_after_timeout);

    register_suite(s);
}

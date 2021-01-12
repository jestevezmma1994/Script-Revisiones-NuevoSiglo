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
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>
#include <check.h>
#include <SvCore/SvCommonDefs.h> // SvConstructor
#include <SvFoundation/SvArray.h>

#include <QBAppKit/QBAsyncService.h>
#include <SvFoundation/SvInterface.h>
#include <QBStaticStorage.h> // is used internaly by JSONserializer
#include <Services/core/JSONserializer.h>

extern void register_suite(Suite *s);

static SvScheduler scheduler = NULL;

// just create an object and destroy to find out that we release all things
START_TEST(QBJSONSerializer_QBJSONSerializerCreate)
{
    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();

    QBJSONSerializer serializer = QBJSONSerializerCreate();
    SVRELEASE(serializer);

    SvSchedulerLoop(scheduler, false);
    SvSchedulerCleanup();
    sleep(1);
    scheduler = NULL;

}
END_TEST

// just start and stop fiber used to serialize
START_TEST(QBJSONSerializer_QBJSONSerializerStartStop_empty_run)
{
    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();

    QBJSONSerializer serializer = QBJSONSerializerCreate();
    SvInvokeInterface(QBAsyncService, serializer, start, scheduler, NULL);
    sleep(1);
    SvInvokeInterface(QBAsyncService, serializer, stop, NULL);
    SVRELEASE(serializer);

    SvSchedulerLoop(scheduler, false);
    SvSchedulerCleanup();
    sleep(1);
    scheduler = NULL;
}
END_TEST

// it should fail because QBJSONSerializer and QBStaticStorage are connected
// it fail because QBStaticStorage do not exist
START_TEST(QBJSONSerializer_QBJSONSerializerAddJob_fail)
{
    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();

    char *cFileName = tempnam("/tmp", "QBJSONSerializer_fail");
    SvString fileName = SvStringCreate(cFileName, NULL);
    SvArray content = SvArrayCreate(NULL);

    QBJSONSerializer serializer = QBJSONSerializerCreate();
    SvInvokeInterface(QBAsyncService, serializer, start, scheduler, NULL);

    bool addedToWrite = QBJSONSerializerAddJob(serializer, (SvObject) content, fileName);
    // job was scheduler but it not be serialized
    ck_assert(addedToWrite == true);

    SvInvokeInterface(QBAsyncService, serializer, stop, NULL);
    SVRELEASE(serializer);

    SvSchedulerLoop(scheduler, false);
    SvSchedulerCleanup();
    sleep(1);
    scheduler = NULL;

    // check file - it should not exist
    FILE *fp = fopen(cFileName, "r");
    ck_assert(fp == NULL);

    free(cFileName);
    SVRELEASE(fileName);
    SVRELEASE(content);
}
END_TEST

// write to file empty SvArray
START_TEST(QBJSONSerializer_QBJSONSerializerAddJob_valid)
{
    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();

    // create QBStaticStorage used by QBJSONSerializer to write data into file
    QBStaticStorageStart(scheduler, 10);

    char *cFileName = tempnam("/tmp", "QBJSONSerializer_vaild");
    SvString fileName = SvStringCreate(cFileName, NULL);
    SvArray content = SvArrayCreate(NULL);

    QBJSONSerializer serializer = QBJSONSerializerCreate();
    SvInvokeInterface(QBAsyncService, serializer, start, scheduler, NULL);

    bool addedToWrite = QBJSONSerializerAddJob(serializer, (SvObject) content, fileName);
    // job was scheduler but it not be serialized
    ck_assert(addedToWrite == true);

    SvInvokeInterface(QBAsyncService, serializer, stop, NULL);
    SVRELEASE(serializer);

    QBStaticStorageStop();
    SvSchedulerLoop(scheduler, false);
    SvSchedulerCleanup();
    sleep(1);
    scheduler = NULL;

    // check file - it should exist
    FILE *fp = fopen(cFileName, "r");
    ck_assert(fp != NULL);

    //TODO: check content of file

    fclose(fp);
    free(cFileName);
    SVRELEASE(fileName);
    SVRELEASE(content);
}
END_TEST

static SvConstructor void init(void)
{
    Suite *s = suite_create("CubiTV");
    TCase *tc = tcase_create("QBJSONSerializer");
    suite_add_tcase(s, tc);

    tcase_add_test(tc, QBJSONSerializer_QBJSONSerializerCreate);
    tcase_add_test(tc, QBJSONSerializer_QBJSONSerializerStartStop_empty_run);
    tcase_add_test(tc, QBJSONSerializer_QBJSONSerializerAddJob_fail);
    tcase_add_test(tc, QBJSONSerializer_QBJSONSerializerAddJob_valid);

    register_suite(s);
}

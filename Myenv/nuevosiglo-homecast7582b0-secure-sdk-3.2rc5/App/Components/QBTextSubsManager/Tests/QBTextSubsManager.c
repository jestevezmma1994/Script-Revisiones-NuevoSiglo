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

#include <string.h> // strlen
#include <stdlib.h> // malloc
#include <dirent.h> // readdir
#include <SvCore/SvCommonDefs.h> // SvConstructor
#include <SvCore/SvErrorInfo.h> // SvErrorInfo
#include <SvFoundation/SvObject.h> // SVRELEASE
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvIterator.h>
#include "../src/QBTTMLSubsParser.h"
#include <check.h>

extern void register_suite(Suite *s);
unsigned char *loadFile(const char* fileName);
SvArray getPathesToTests(const char* directory);

#define DIRECTORY_WITH_W3M_TESTS "data/TTML/valid/external/w3m"
#define DIRECTORY_WITH_QB_TESTS "data/TTML/valid/qb"

// only create - check that we have valid destructor
START_TEST(Test_QBTTMLSubsParserCreate)
{
    SvErrorInfo error = NULL;
    QBTTMLSubsParser parser = QBTTMLSubsParserCreate(&error);
    ck_assert(error == NULL);
    SVRELEASE(parser);
}
END_TEST

// parser valid data - set of test from w3m
START_TEST(Test_QBTTMLSubsParser_valid_w3m_tests)
{
    SvArray tests = getPathesToTests(DIRECTORY_WITH_W3M_TESTS);
    SvIterator iter = SvArrayGetIterator(tests);
    SvString testPathFile = NULL;

    while ((testPathFile = (SvString) SvIteratorGetNext(&iter))) {
        unsigned char *data = loadFile(SvStringCString(testPathFile));

        size_t length = (size_t) strlen((char*) data);
        SvErrorInfo error = NULL;
        QBTTMLSubsParser parser = QBTTMLSubsParserCreate(&error);
        ck_assert(error == NULL);

        QBTTMLSubsParserParse(parser, data, length, &error);
        ck_assert(error == NULL);

        SVRELEASE(parser);
        free(data);
    }

    SVRELEASE(tests);
}
END_TEST

// parser valid data - QB tests
START_TEST(Test_QBTTMLSubsParser_valid_qb_tests)
{
    SvArray tests = getPathesToTests(DIRECTORY_WITH_QB_TESTS);
    SvIterator iter = SvArrayGetIterator(tests);
    SvString testPathFile = NULL;

    while ((testPathFile = (SvString) SvIteratorGetNext(&iter))) {
        unsigned char *data = loadFile(SvStringCString(testPathFile));

        size_t length = (size_t) strlen((char*) data);
        SvErrorInfo error = NULL;
        QBTTMLSubsParser parser = QBTTMLSubsParserCreate(&error);
        ck_assert(error == NULL);

        QBTTMLSubsParserParse(parser, data, length, &error);
        ck_assert(error == NULL);

        SVRELEASE(parser);
        free(data);
    }

    SVRELEASE(tests);
}
END_TEST

// instead of destroing parser after parse data we reset it
START_TEST(Test_QBTTMLSubsParserReset_on_valid_data_w3m)
{
    SvArray tests = getPathesToTests(DIRECTORY_WITH_W3M_TESTS);
    SvIterator iter = SvArrayGetIterator(tests);
    SvString testPathFile = NULL;
    SvErrorInfo error = NULL;

    QBTTMLSubsParser parser = QBTTMLSubsParserCreate(&error);
    ck_assert(error == NULL);

    while ((testPathFile = (SvString) SvIteratorGetNext(&iter))) {
        unsigned char *data = loadFile(SvStringCString(testPathFile));
        size_t length = (size_t) strlen((char*) data);

        QBTTMLSubsParserParse(parser, data, length, &error);
        ck_assert(error == NULL);

        QBTTMLSubsParserReset(parser);
        free(data);
    }
    SVRELEASE(parser);
    SVRELEASE(tests);
}
END_TEST

static SvConstructor void init(void)
{
    Suite *s  = suite_create("QBTextSubsManager");
    TCase *tc = tcase_create("QBTTMLSubsParser");
    suite_add_tcase(s, tc);

    tcase_add_test(tc, Test_QBTTMLSubsParserCreate);
    tcase_add_test(tc, Test_QBTTMLSubsParser_valid_w3m_tests);
    tcase_add_test(tc, Test_QBTTMLSubsParser_valid_qb_tests);
    tcase_add_test(tc, Test_QBTTMLSubsParserReset_on_valid_data_w3m);

    register_suite(s);
}


SvArray getPathesToTests(const char* directory)
{
    SvArray tests = SvArrayCreate(NULL);
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type != DT_REG)
                continue; // ommit '.', '..'
            SvString testPath = SvStringCreateWithFormat("%s/%s", directory, dir->d_name);
            SvArrayAddObject(tests, (SvObject) testPath);
            SVRELEASE(testPath);
        }
        closedir(d);
    }
    return tests;
}

unsigned char* loadFile(const char* fileName)
{
    FILE *fp;
    long lSize;
    unsigned char *buffer;
    fp = fopen((const char *) fileName, "rb");
    if (!fp) {
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    buffer = calloc(1, lSize + 1);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    if (fread(buffer, lSize, 1, fp) != 1) {
        fclose(fp);
        free(buffer);
        return NULL;
    }
    fclose(fp);
    return buffer;
}

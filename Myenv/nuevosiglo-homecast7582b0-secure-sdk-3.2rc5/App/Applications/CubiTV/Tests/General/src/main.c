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

#include "suites/testSuites.h"
#include "suites/common.h"
#include <QBConfig.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <SvCore/SvLog.h>
#include <fibers/c/fibers.h>

//Add test suites in this method
static void RunAllTests(const char * output)
{
    //Init config file access
    QBConfigInit();

    //Create scheduler for handling application fibers
    SvSchedulerCreateMain();

    SvErrorInfo error = NULL;
    QBTests_startPythonServer("server-qbaccessmanagertest.py", 1, &error);
    assert(error == NULL);

    SRunner *sr = srunner_create(QBCubiTv_QBAcccessManager_testSuite());
    srunner_set_xml(sr, output);
    srunner_run_all(sr, CK_VERBOSE);
    srunner_free(sr);

    QBTests_stopPythonServer();

    //destroy scheduler
    SvSchedulerDestroy(SvSchedulerGet());

    //deinit config
    QBConfigDestroy();
}

int main(int argc, char *argv[])
{
    char *outputPath = NULL;
    asprintf(&outputPath, "%s.check.xml", argv[0]);
    if (argc > 1) {
        char *tmp = NULL;
        asprintf(&tmp, "%s/%s", argv[1], outputPath);
        free(outputPath);
        outputPath = tmp;
    }

    RunAllTests(outputPath);

    free(outputPath);

    return 0;
}

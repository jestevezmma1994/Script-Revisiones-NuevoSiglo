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


#include "common.h"
#include <check.h>
#include <signal.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/core/QBDebugUtils.h>
#include <QBConfig.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBInitializable.h>
#include <SvCore/SvLog.h>
#include <fibers/c/fibers.h>
#include <main.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define CUBITV_TEST_LONG "CUBITV_TEST_LONG"

bool QBTestsLongTestsEnabled(void)
{
    char* envVal = getenv(CUBITV_TEST_LONG);
    if (envVal != NULL && (strcmp(envVal, "yes") == 0 || strcmp(envVal, "true") == 0 || strcmp(envVal, "1") == 0)) {
        return true;
    }
    return false;
}

SvLocal int serverPort = 0;

void QBAccessManagerTest_initMinimalApplication(AppGlobals appGlobals)
{
    //Enable debugging printing (all SvObjects memory leaks)
    QBDebugUtilsRegisterHooks();
    setenv("SV_FOUNDATION_DEBUG", "1", 1);

    SvErrorInfo error = NULL;
    //Initialize ssl connection parameters instance
    appGlobals->sslParams = (SvSSLParams) SvTypeAllocateInstance(SvSSLParams_getType(), &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));
    SvSSLParamsInit(appGlobals->sslParams, SvSSLVersion_default, &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));
    SvSSLParamsSetClientAuth(appGlobals->sslParams, "/etc/certs/private/key.pem", SvSSLFileFormat_PEM,
                             "/etc/certs/private/cert.pem", SvSSLFileFormat_PEM,
                             &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));
    SvSSLParamsSetVerificationOptions(appGlobals->sslParams, true, true, "/etc/certs/public/cacert.pem",
                                      &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));

    //set the CUSTOMER_ID for INNOV8ON access plugin
    setenv("FAKE_INNOV8ON_CUSTOMER_ID", "SCID!12345678901", 1);

    //set the server url (this will be our fake python server)
    SvString cmdLineUrl = SvStringCreateWithFormat("http://localhost:%d", serverPort);
    SvString prefixUrl = SvStringCreateWithFormat("http://localhost:%d", serverPort);
    QBMiddlewareManagerInitParams params = {
        .redirProxyEnabled = false,
        .prefixUrl         = prefixUrl,
        .cmdLineUrl        = cmdLineUrl,
        .sslParams         = appGlobals->sslParams
    };

    //Initialize middleware manager (for INNOV8ON plugin)
    appGlobals->middlewareManager = QBMiddlewareManagerCreate();
    QBMiddlewareManagerInit(appGlobals->middlewareManager, &params);

    SvLogNotice("Initializing middleware manager - done");

    SVRELEASE(cmdLineUrl);
    SVRELEASE(prefixUrl);

    appGlobals->scheduler = SvSchedulerGet();

    //Allocate and initialize QBAccessManager instance
    appGlobals->accessMgr = (QBAccessManager) SvTypeAllocateInstance(QBAccessManager_getType(), &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));
    SvInvokeInterface(QBInitializable, appGlobals->accessMgr, init, NULL, &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));
    SvInvokeInterface(QBAsyncService, appGlobals->accessMgr, start, appGlobals->scheduler, &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));

    //Make shure that no pin block is in action
    QBAccessManagerResetPinAttemptsLeft(appGlobals->accessMgr);
}

void QBAccessManagerTest_deinitMinimalApplication(AppGlobals appGlobals)
{
    SvErrorInfo error = NULL;

    //Stop QBAccessManager
    SvInvokeInterface(QBAsyncService, appGlobals->accessMgr, stop, &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));
    SVTESTRELEASE(appGlobals->accessMgr);
    appGlobals->accessMgr = NULL;

    //free Middleware manager
    SVTESTRELEASE(appGlobals->middlewareManager);
    appGlobals->middlewareManager = NULL;

    //free ssl params
    SVTESTRELEASE(appGlobals->sslParams);
    appGlobals->sslParams = NULL;
}

SvLocal size_t get_executable_path(char* buffer, size_t size)
{
    char* end_ptr = NULL;
    if (readlink("/proc/self/exe", buffer, size) <= 0)
        return -1;
    end_ptr = strrchr(buffer, '/');
    if (end_ptr == NULL)
        return -1;
    ++end_ptr;
    *end_ptr = '\0';
    return (size_t) (end_ptr - buffer);
}

SvLocal int runningServer = -1;

void QBTests_startPythonServer(const char* scriptFile, double serverAwaitTimeSecs, SvErrorInfo *errorOut)
{
    serverPort = 0;
    SvErrorInfo error = NULL;
    QBTests_stopPythonServer();

    int filedes[2];
    if (pipe(filedes) == -1) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Unable to start server process - cannot create pipe");
        goto err;
    }

    char path_buff[1024];
    memset(path_buff, 0, sizeof(path_buff));
    size_t pathL = get_executable_path(path_buff, sizeof(path_buff));
    if (pathL == (size_t) -1) {
        exit(0);
    }

    int childId = fork();
    if (childId < 0) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Unable to start server process - fork did not succeed");
        close(filedes[0]);
        close(filedes[1]);
        goto err;
    }
    if (childId == 0) {
        while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {
        }
        close(filedes[1]);
        close(filedes[0]);
        char* serverCommand = NULL;
        asprintf(&serverCommand, "%s%s", path_buff, scriptFile);
        SvLogNotice("system command: %s", serverCommand);
        int status = execlp("python", "python", serverCommand, NULL);
        SvLogError("system returned: %d for command %s", status, serverCommand);
        free(serverCommand);
        exit(0);
    }
    close(filedes[1]);
    char buffer[1024];
    while (1) {
        ssize_t count = read(filedes[0], buffer, sizeof(buffer));
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        } else if (count == 0) {
            break;
        } else {
            serverPort = atoi(buffer);
            break;
        }
    }
    close(filedes[0]);
    if (serverPort == 0) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Unable to read server perot number - maybe the server did no raport it");
        goto err;
    }
    runningServer = childId;
    if (serverAwaitTimeSecs > 0) {
        unsigned int microseconds = (unsigned int) (serverAwaitTimeSecs * 1000000);
        usleep(microseconds);
    }
    SvLogNotice("Started python server: %d on port %d", runningServer, serverPort);
err:
    SvErrorInfoPropagate(error, errorOut);
}

void QBTests_stopPythonServer(void)
{
    if (runningServer > 0) {
        kill(runningServer, SIGKILL);
        SvLogNotice("Killed python server: %d", runningServer);
        runningServer = -1;
    }
}

SvObject safeCreateTestObject(SvType type)
{
    SvErrorInfo error = NULL;
    SvObject object = NULL;
    object = SvTypeAllocateInstance(type, &error);
    ck_assert_msg(error == NULL, SvErrorInfoGetMessage(error));
    return object;
}

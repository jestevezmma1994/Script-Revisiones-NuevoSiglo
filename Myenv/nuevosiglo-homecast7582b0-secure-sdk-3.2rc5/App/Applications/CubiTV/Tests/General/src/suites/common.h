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

#ifndef COMMON_CUBITV_TESTS_H
#define COMMON_CUBITV_TESTS_H

/**
 * @file common.h Common functions for general CubiTV tests
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <check.h>
#include <stdbool.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>
#include <main_decl.h>

/**
 * @defgroup CommonTestUtils Common utils for unit tests of Cubitv
 * @ingroup CubiTV_tests
 * @{
 */

/**
 * Checks if env CUBITV_TEST_LONG is set to yes, true or 1
 *
 * @return true if CUBITV_TEST_LONG is set to yes, true or 1, and false otherwise
 */
bool QBTestsLongTestsEnabled(void);

/**
 * Initialize minimal application for QBAccesManager tests
 *
 * @param[in] appGlobals    appGlobals pointer to be setup.
 **/
void QBAccessManagerTest_initMinimalApplication(AppGlobals appGlobals);

/**
 * Deinitialize minimal application for QBAccesManager tests
 *
 * @param[in] appGlobals    appGlobals pointer to be uninitialized (same as passed to QBAccessManagerTest_initMinimalApplication).
 **/
void QBAccessManagerTest_deinitMinimalApplication(AppGlobals appGlobals);

/**
 * Spawns a python process, which will execute a script passed as parameter.
 * Script should reside in the same directory as the test executable directory.
 * Spawned process ID will be remembered internally for the QBTests_stopPythonServer function.
 *
 * @param[in] scriptFile                name of script file to execute
 * @param[in] serverAwaitTimeSecs       amount of time (seconds) which this process will wait
 *                                      after spawning a python process before continuing
 *                                      (time for server to start serving requests).
 * @param[out] errorOut                 error handle
 **/
void QBTests_startPythonServer(const char* scriptFile, double serverAwaitTimeSecs, SvErrorInfo *errorOut);

/**
 * Stops python process spawned py the QBTests_startPythonServer function
 **/
void QBTests_stopPythonServer(void);

/**
 * Calls method SvTypeAllocateInstance in a safe manner. Asserts on success of allocation (ck_assert)
 *
 * @param[in]                           type  type of object to be created
 * @return                              properly allocated object
 **/
SvObject safeCreateTestObject(SvType type);

/**
 * Macro for wrapping a call to any function that handles error via SvErrorInfo.
 * This macro just calls the code passed as parameter and asserts that there was no error.
 **/
#define __SAFE_CALL__(code) { \
        SvErrorInfo __error__ = NULL; \
        SvErrorInfo* _error_ = &__error__; \
        code \
        ck_assert_msg(__error__ == NULL, SvErrorInfoGetMessage(__error__)); \
}

/**
 * @}
 **/

#endif //COMMON_CUBITV_TESTS_H

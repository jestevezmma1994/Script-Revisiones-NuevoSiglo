/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau,
 ** application service provider, or similar business, or make any other use of
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ****************************************************************************/

#ifndef QBUSBPERFTESTER_H_
#define QBUSBPERFTESTER_H_

/**
* @file QBUSBPerfTester.h
* @brief USB storage devices performance tester
* @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
* @endxmlonly
**/

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <stdbool.h>

/**
* @defgroup QBUSBPerfTester  QBUSBPerfTester class
* @{
**/

/**
* QBUSBPerfTester class
*
* Some of USB devices are faster and some are slower. This tester is used to check what user can do with inserted USB stage device.
* It simulates PVR recording and check how many simultaneous SD/HD recordings device can handle. In each test CPU usage is also logged
* and it used to determine if STB can handle given number of recordings.
*
* @class QBUSBPerfTest
*/
typedef struct QBUSBPerfTest_* QBUSBPerfTest;

/**
 * Check if during execution of test time shift will be simulated
 *
 * @param[in] self QBUSBPerfTest instance
 * @return @c true if timeshift is testedm @c false otherwise
 */
bool QBUSBPerfTestHasTS(QBUSBPerfTest self);

/**
 * Get number of simulated SD recordings during test
 *
 * @param[in] self QBUSBPerfTest instance
 * @return number of tested sd recordings
 */
unsigned int QBUSBPerfTestGetSDCount(QBUSBPerfTest self);

/**
 * Get number of simulated HD recordings during test
 *
 * @param[in] self QBUSBPerfTest instance
 * @return number of tested hd recordings
 */
unsigned int QBUSBPerfTestGetHDCount(QBUSBPerfTest self);

/**
 * Get test specific string
 *
 * @param[in] self QBUSBPerfTest instance
 * @param[in] id string ID
 * @return test specific string
 */
SvString QBUSBPerfTestGetString(QBUSBPerfTest self, SvString id);


/**
 *
 * @class QBUSBPerfTester
 */
typedef struct QBUSBPerfTester_* QBUSBPerfTester;

struct QBUSBPerfTestResult_ {
    /**
     *  Number of bytes that should be read during test
     */
    unsigned bytesToRead;
    /**
     *  Number of bytes that should be wrote during test
     */
    unsigned bytesToWrite;
    /**
     *  Actual number of bytes read in test
     */
    unsigned read;
    /**
     *  Actual number of bytes wrote in test
     */
    unsigned wrote;
    /**
     * True if test passed, false otherwise
     */
    bool passed;
    /**
     * Value in range [0, testCnt-1]
     */
    unsigned testNumber;
};

/**
 *  Result of single test
 */
typedef struct QBUSBPerfTestResult_ QBUSBPerfTestResult;

struct QBUSBPerfTesterListener_t {
    /**
     * Called when single test ended
     * @param self_ listener instance
     * @param result information about test
     * @param testId test ID
     */
    void (*testFinished)(SvObject self_, QBUSBPerfTestResult testFinished);
    /**
     * Called when single test started
     * @param self_ listener instance
     * @param test test
     */
    void (*testStarted)(SvObject self_, QBUSBPerfTest test);
    /**
     * Called when no more test will be started
     *
     * @param self_ listener instance
     * @param tests array with all tests
     * @param highestPassedTestId ID of highest passed test or value < 0 on error
     */
    void (*allTestsFinished)(SvObject self_, SvArray tests, int highestPassedTestId);
};
typedef struct QBUSBPerfTesterListener_t* QBUSBPerfTesterListener;

/**
 * Create QBUSBPerfTester instance
 *
 * @param[in] diskId     disk id
 * @param[in] devicePath path to device
 * @param[in] testPath   path to tests
 * @return created QBUSBPerfTester instance
 */
QBUSBPerfTester QBUSBPerfTesterCreate(SvString diskId, SvString devicePath, SvString testPath);

/**
 * Start test procedure (all tests will be started, one by one)
 *
 * @param[in] self QBUSBPerfTester instance
 */
void QBUSBPerfTesterStart(QBUSBPerfTester self);

/**
 * Force end of the test
 *
 * @param[in] self QBUSBPerfTester instance
 */
void QBUSBPerfTesterStop(QBUSBPerfTester self);

/**
 *  Get runtime type identification object representing QBUSBPerfTesterListener interface
 *  @return USBPerf Tester listener interface handle
 **/
SvInterface QBUSBPerfTesterListener_getInterface(void);

/**
 * Adds a new listener that will be notified about test progress.
 * Listener must implement the @ref QBUSBPerfTesterListener.
 *
 * @param[in] self QBUSBPerfTester instance
 * @param[in] listener listener object handle
 */
void QBUSBPerfTesterAddListener(QBUSBPerfTester self, SvObject listener);

/**
 * Remove listener from USB performance tester
 *
 * @param[in] self QBUSBPerfTester instance
 * @param[in] listener listener object handle
 */
void QBUSBPerfTesterRemoveListener(QBUSBPerfTester self, SvObject listener);

/**
 * Get number of tests scheduled
 *
 * @param[in] self QBUSBPerfTester instance
 * @return number of tests scheduled
 */
unsigned QBUSBPerfTesterGetTestsCount(QBUSBPerfTester self);

/**
 * Get disk id
 *
 * @param[in] self QBUSBPerfTester instance
 * @return disk id
 */
SvString QBUSBPerfTesterGetDiskId(QBUSBPerfTester self);

/**
 * Get test parameters for test id
 *
 * @param[in]  self QBUSBPerfTester instance
 * @param[in]  id test identificator
 * @param[out] sdCnt number of SD recordings
 * @param[out] hdCnt number of HD recordings
 * @param[out] ts determines if timeshift used in test
 * @return value < 0 on error, >= 0 on success
 */
int QBUSBPerfTesterGetTestParamsForId(QBUSBPerfTester self, SvString id, unsigned *sdCnt, unsigned *hdCnt, bool *ts);


/**
* @}
*/

#endif

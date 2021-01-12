/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBMP4PARSERENGINE_H_
#define QBMP4PARSERENGINE_H_

/**
 * @file QBMP4ParserEngine.h
 * @brief MP4 Parser Engine API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <storage_input/storage_input.h>
#include <fibers/c/fibers.h>

/**
 * @defgroup QBMP4ParserEngine MP4 Parser Engine class
 * @{
 **/

/**
 * Specifies mode of MP4 Parser Engine
 **/
typedef enum {
    QBMP4ParserEngineMode_unknown = -1, ///< special marker when mode is unknown
    QBMP4ParserEngineMode_file,         ///< for file MP4
    QBMP4ParserEngineMode_fragmented    ///< for fragmented MP4
} QBMP4ParserEngineMode;

/**
 * Possible statuses returned from QBMP4ParserEngine methods
 **/
typedef enum {
    QBMP4ParserEngineStatus_unknown = -1, ///< special marker when status is unknown
    QBMP4ParserEngineStatus_ok,           ///< success
    QBMP4ParserEngineStatus_error         ///< error
} QBMP4ParserEngineStatus;

/**
 * Specifies state of MP4 Parser Engine
 **/
typedef enum {
    QBMP4ParserEngineState_unknown = -1,       ///< special marker when state is unknown
    QBMP4ParserEngineState_initial,            ///< initial state
    QBMP4ParserEngineState_processingHeaders,  ///< processing MP4 data headers
    QBMP4ParserEngineState_processingFrames,   ///< processing MP4 data frames
    QBMP4ParserEngineState_paused,             ///< processing is paused
    QBMP4ParserEngineState_finished            ///< parsing is finished
} QBMP4ParserEngineState;

/**
 * Specifies error list in MP4 Parser Engine
 **/
typedef enum {
    QBMP4ParserEngineError_unknown = -1, ///< special marker when error is unknown
    QBMP4ParserEngineError_inputError,   ///< storage input error
    QBMP4ParserEngineError_badFormat     ///< bad format of MP4 container
} QBMP4ParserEngineError;

/**
 * MP4 Parser Engine class.
 * This class is responsible for processing MP4 container.
 *
 * @class QBMP4ParserEngine
 * @extends SvObject
 **/
typedef struct QBMP4ParserEngine_ *QBMP4ParserEngine;

/**
 * Get runtime type identification object representing
 * type of MP4 Parser Engine.
 *
 * @return MP4 Parser Engine class
 **/
extern SvType
QBMP4ParserEngine_getType(void);

/**
 * Create MP4 Parser Engine.
 *
 * @param[in] mode      mode of MP4 Parser Engine
 * @param[in] scheduler application scheduler handle
 * @param[out] errorOut error info
 * @return              new created instance of QBMP4ParserEngine
 **/
extern QBMP4ParserEngine
QBMP4ParserEngineCreate(QBMP4ParserEngineMode mode,
                        SvScheduler scheduler,
                        SvErrorInfo *errorOut);

/**
 * Start MP4 Parser Engine.
 *
 * @param[in]  self     MP4 Parser Engine handle
 * @param[in]  input    storage input which is data source
 * @param[out] errorOut error info
 * @return              MP4 Parser Engine status code
 **/
extern QBMP4ParserEngineStatus
QBMP4ParserEngineStart(QBMP4ParserEngine self,
                       struct storage_input *input,
                       SvErrorInfo *errorOut);

/**
 * Pause MP4 frames processing.
 *
 * @param[in]  self     MP4 Parser Engine handle
 * @param[out] errorOut error info
 * @return              MP4 Parser Engine status code
 **/
extern QBMP4ParserEngineStatus
QBMP4ParserEnginePauseFrameProcessing(QBMP4ParserEngine self,
                                      SvErrorInfo *errorOut);

/**
 * Resume MP4 frames processing.
 *
 * @param[in]  self     MP4 Parser Engine handle
 * @param[out] errorOut error info
 * @return              MP4 Parser Engine status code
 **/
extern QBMP4ParserEngineStatus
QBMP4ParserEngineResumeFrameProcessing(QBMP4ParserEngine self,
                                       SvErrorInfo *errorOut);

/**
 * Seek to given position.
 *
 * @param[in]  self     MP4 Parser Engine handle
 * @param[in]  pts90k   position in 90kHz units
 * @param[out] errorOut error info
 * @return              position in 90kHz units
 **/
extern uint64_t
QBMP4ParserEngineSeek(QBMP4ParserEngine self,
                      uint64_t pts90k,
                      SvErrorInfo *errorOut);

/**
 * Cancel MP4 headers/frames processing.
 *
 * @param[in]  self     MP4 Parser Engine handle
 * @param[out] errorOut error info
 * @return              MP4 Parser Engine status code
 **/
extern QBMP4ParserEngineStatus
QBMP4ParserEngineCancelProcessing(QBMP4ParserEngine self,
                                  SvErrorInfo *errorOut);

/**
 * Reset MP4 Parser Engine.
 *
 * @param[in]  self     MP4 Parser Engine handle
 * @param[out] errorOut error info
 * @return              MP4 Parser Engine status code
 **/
extern QBMP4ParserEngineStatus
QBMP4ParserEngineReset(QBMP4ParserEngine self,
                       SvErrorInfo *errorOut);

/**
 * Get state of MP4 Parser Engine.
 *
 * @param[in]  self     MP4 Parser Engine handle
 * @return              MP4 Parser Engine state
 **/
extern QBMP4ParserEngineState
QBMP4ParserEngineGetState(QBMP4ParserEngine self);

/**
 * Set listener to MP4 Parser Engine.
 *
 * Listener must implement @ref QBMP4ParserEngineListener
 *
 * @param[in]  self      MP4 Parser Engine handle
 * @param[in]  listener  listener
 * @param[out] errorOut  error info
 * @return               MP4 Parser Engine status
 **/
extern QBMP4ParserEngineStatus
QBMP4ParserEngineSetListener(QBMP4ParserEngine self,
                             SvObject listener,
                             SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMP4PARSERENGINE_H_ */

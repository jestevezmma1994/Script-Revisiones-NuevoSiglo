/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCAS_H_
#define QBCAS_H_

/**
 * @file QBCAS.h QBCAS interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvData.h>
#include <dataformat/sv_data_format.h>
#include <dataformat/QBContentSource.h>
#include <fibers/c/fibers.h>
#include <QBDescrambler/basic_types.h>
#include <QBCASInfo.h>
#include <QBCASPVRSettings.h>
#include <QBTSSectionCustomFilter.h>
#include <QBCASKeyManagerMeta.h>
#include <SvCore/SvTime.h>
#include <stdbool.h>

typedef struct QBCAS_i* QBCAS;

/** Maximum allowed length of EMM filter */
#define QBCAS_MAX_EMMFILTER_LEN 10

#define QBCAS_INVALID_SESSION_ID -1

/// CAS SYSTEM_ID used in our internal content encryption
#define QBCAS_INTERNAL_CA_SYSTEM_ID 0x0000
/**
 * QBCAS command structure declaration, used only as a pointer to internal CAS command structure
 **/
typedef struct QBCASCmd_s * QBCASCmd;

/**
 * QBCAS PushECM error codes.
 **/
typedef enum {
    /** ECM was matched - it was queued for futher processing (callback will be called when done) */
    QBCASPushECMResponseCode_Queued = 0,
    /** ECM was ignored */
    QBCASPushECMResponseCode_Ignored = 1,
    /** ECM was matched, but callback won't be called (used by CAS implementations which don't support callbacks) */
    QBCASPushECMResponseCode_QueuedNoCallback = 2,
    /** Error occurred during processing ECM */
    QBCASPushECMResponseCode_Error = -1,
} QBCASPushECMResponseCode;

/**
 * QBCAS message format
 */
typedef enum {
    QBCASMessageFormat_Section,/**< Parse selected pid to retrieve data in section format */
    QBCASMessageFormat_Packet, /**< Do not parse selected pid, just send whole packets. */
    QBCASMessageFormat_Max,    /**< QBCASEMMFormat_Max */
} QBCASMessageFormat;

/**
 * EMM filter
 */
struct QBCASEMMFilter_s
{
    int pid; /**< EMM pid */
    QBCASMessageFormat format;

    bool use_custom_filter; /**< if true below custom_filter will be used as a section filter */
    union
    {
        struct
        {
            int length; /**< length of data and mask to use in filter */
            uint8_t data[QBCAS_MAX_EMMFILTER_LEN];
            uint8_t mask[QBCAS_MAX_EMMFILTER_LEN];
        } filter;
        struct
        {
            QBTSSectionCustomFilterFun* callback; /**< callback to be used as a custom section filter */
            void* target; /**< custom section filter context */
        } customFilter;
    };
};

/**
 * QBCASEMMFilter structure describing single emm filter.
 */
typedef struct QBCASEMMFilter_s *QBCASEMMFilter;

/**
 * Possible smartcard states
 */
enum QBCASSmartcardState_e
{
    QBCASSmartcardState_removed,
    QBCASSmartcardState_inserted,
    QBCASSmartcardState_correct,
    QBCASSmartcardState_incorrect_ex,
    QBCASSmartcardState_incorrect,
    QBCASSmartcardState_problem,
};
typedef enum QBCASSmartcardState_e  QBCASSmartcardState;

/**
 * Possible reencryption info
 */
enum QBCASReencryptionInfo_e {
    QBCASReencryptionInfo_broadcasterLimitation,
    QBCASReencryptionInfo_noAccess,
};
typedef enum QBCASReencryptionInfo_e QBCASReencryptionInfo;

/**
 * Possible status of ECM processing
 */
enum QBCASEcmStatus_e
{
    QBCASEcmStatus_unknown,
    QBCASEcmStatus_ok,
    QBCASEcmStatus_no_cw,
    QBCASEcmStatus_ignored,
    QBCASEcmStatus_failed,
    QBCASEcmStatus_failed_external,
    QBCASEcmStatus_failed_no_cat,

};
typedef enum QBCASEcmStatus_e  QBCASEcmStatus;

enum QBCASIndividualizationState_e
{
    QBCASIndividualizationState_notNeeded,
    QBCASIndividualizationState_individualizationNeeded,
};
typedef enum QBCASIndividualizationState_e  QBCASIndividualizationState;

enum QBCASIndividualizationInfo_e
{
    QBCASIndividualizationInfo_success,
    QBCASIndividualizationInfo_error,
    QBCASIndividualizationInfo_successRebootNeeded,
};
typedef enum QBCASIndividualizationInfo_e  QBCASIndividualizationInfo;

/**
 * Possible session types
 */
enum QBCASSessionType_e {
    /// Type of session is unknown
    QBCASSessionType_unknown,
    /// Session is used for playback
    QBCASSessionType_playback,
    /// Session is used for reencryption
    QBCASSessionType_reencryption,
};
typedef enum QBCASSessionType_e QBCASSessionType;

/**
 * Generic CAS Callback
 * @param[in] arg private data
 * @param[in] cmd ptr to command - the same that returned in function that scheduled the callback
 * @param[in] status operation status
 */
typedef void (QBCASCallbackFun)        (void* arg, QBCASCmd cmd, int status);

/**
 * Callback called when CAT is parsed
 * @param[in] arg private data
 * @param[in] cnt number of EMM filters, -1 means that parsing CAT failed
 * @param[in] emm_filters EMM filters table
 */
typedef void (QBCASCallbackCatFun)     (void* arg, int cnt, const struct QBCASEMMFilter_s* emm_filters);

/**
 * Callback contains CAS info
 * @param[in] arg private data
 * @param[in] cmd ptr to command - the same that returned in function that scheduled the callback
 * @param[in] status operation status
 * @param[in] info actual CAS info
 */
typedef void (QBCASCallbackInfoFun)    (void* arg, const QBCASCmd cmd, int status, QBCASInfo info);


struct QBCASCallbacks_s {
    /**
     * Callback called when CAS initialization is done
     * @param[in] target callback private data
     * @param[in] success result of initialization
     */
    void (*init_done)(void* target, bool success);

    /**
     * Callback called when CAS smartcard state is changed
     * @param[in] target callback private data
     * @param[in] state new smartcard state
     */
    void (*smartcard_state)(void* target, QBCASSmartcardState state);

    /**
     * Callback called when ECM processing is finished
     * @param[in] target callback private data
     * @param[in] sessionId
     * @param[in] ecmPid ecm pid from which ecm was originated
     * @param[in] status ECM processing status
     * @param[in] is_extra_ecm
     * @param[in] no_access_code
     * @param[in] cw Control Words or NULL if not present
     */
    void (*ecm_status)(void* target, uint8_t sessionId, int16_t ecmPid, QBCASEcmStatus status, bool is_extra_ecm, int no_access_code, const QBDescramblerCW* cw);

    /**
     * Callback called when CAS needs new CAT
     */
    void (*want_fresh_cat)(void* target);

    /**
     * Callback called when CAS needs to update emm filter on particular tuner
     * @param[in] target callback private data
     * @param[in] cnt filter count, @c 0 if filtering should be disabled
     * @param[in] emm_filters array with emm filters
     * @param[in] tuner_num ID of the tuner, the same as passed in PushCAT
     */
    void (*update_emm_tuner_filter)(void *target, int cnt, const struct QBCASEMMFilter_s* emm_filters, int tuner_num);

    /**
     * Callback called when new reencryption ECM is created. It contains data which need to be returned to CAS by pushECM
     * while playing back reencrypted conented.
     * @param[in] target callback private data
     * @param[in] sessionId  CAS session ID
     * @param[in] pid ES pid for which ECM was generated
     * @param[in] section buffer with ECM section data
     */
    void (*popECM)(void* target, int8_t sessionId, int pid, SvData section);

    /**
     * Callback called when CAS reencryption state is changed
     * @param[in] target callback private data
     * @param[in] sessionId CAS session ID
     * @param[in] info information about reencryption
     */
    void (*reencryptionInfo)(void* target, int8_t sessionId, QBCASReencryptionInfo info);

    /**
     * Callback called when CAS needs to poll standby state
     * @param[in] target callback private data
     */
    void (*want_standby_state)(void* target);

    /**
     * Callback called when CAS needs to reboot the STB
     * @param[in] target callback private data
     */
    void (*want_to_reboot)(void* target);

    /**
     * Callback called when CAS needs to restore the STB to factory defaults
     * @param[in] target callback private data
     */
    void (*want_factory_reset)(void *target);
};
typedef const struct QBCASCallbacks_s *QBCASCallbacks;

struct QBCASAdditionalInitParams_s {
    unsigned int width;     /**< Width of the application's drawing area */
    unsigned int height;    /**< Height of the application's drawing area */
    int tuners_cnt;         /**< Number of tuners available in the system */
    QBCASKeyManagerMeta metaKeyManager; /**< Manager of keys assigned to HDD */
    struct {
        const char* address; /**< Address of CAS server */
        int port;            /**< Port of CAS server */
    } casServer;
    const char* casDataDirectory; /**< Location of directory where CAS stores it's data */
    bool reencryptionEnabled; /**< @c true if reencryption should be enabled @c false if not */
};
typedef struct QBCASAdditionalInitParams_s *QBCASAdditionalInitParams;

/**
 * QBCAS interface
 * Note: All functions can be called from different threads!
 */
struct QBCAS_i {
    /**
     * Initializes CAS implementation
     * @param[in] self_ QBCAS instance
     * @param[in] scheduler scheduler
     * @param[in] additionalParams additional init params (can't be NULL)
     */
    int (*init)(SvObject self_, SvScheduler scheduler, QBCASAdditionalInitParams additionalParams);

    /**
     * Deinitializes CAS initialization
     */
    void (*deinit)(SvObject self_);

    /**
     * Starts CAS service
     * @param[in] self_ QBCAS instance
     */
    void (*start)(SvObject self_);

    /**
     * Stops CAS service
     * @param[in] self_ QBCAS instance
     */
    void (*stop)(SvObject self_);

    /**
     * TODO: get rid of this an use SvObjectIsInstanceOf()
     * Returns CAS implementation type
     * @param[in] self_ QBCAS instance
     * @return CAS type
     */
    QBCASType (*getCASType)(SvObject self_);

    /**
     * Returns current smartcard state
     */
    QBCASSmartcardState (*getState)(SvObject self_);

    /**
     * Requests start of descrambling/rescrambling process
     *
     * @param[in] self_ QBCAS instance
     * @param[in] fun callback to call when processing is done
     * @param[in] arg argument to pass in callback
     * @param[out] cmd_out CAS command handling this request (filled only if CAS support commands)
     * @param[out] sessionId Session ID assigned to created session
     * @param[in] type of session @see QBCASSessionType
     * @param[in] format information about format of content
     * @param[in] source information about source of content
     */
    int (*startDecryption)(SvObject self_,
                           QBCASCallbackFun *fun, void *arg, QBCASCmd *cmd_out,
                           int8_t *sessionId, QBCASSessionType type,
                           struct svdataformat *format, QBContentSource source);

    /**
     * Requests stop of descrambling/rescrambling process
     */
    int (*stopDecryption)(SvObject self_, int8_t sessionId);

    /*
     * Update CA information from new dataformat
     *
     * @param[in] self_ QBCAS instance
     * @param[in] sessionId id of session
     * @param[in] format new information about format of content
     * @param[in] source information about source of content
     */
    int (*updateDecryption)(SvObject self_, int8_t sessionId, struct svdataformat *format, QBContentSource source);


    /**
     * Flushes current descrambling/rescrambling process
     */
    int (*flushDecryption)(SvObject self_, int8_t sessionId, bool flushCWs);

    /**
     * Handles new CAT
     * @param[in] self_ QBCAS instance
     * @param[in] fun callback to call when CAT is parsed
     * @param[in] arg argument to pass in callback
     * @param[out] cmd_out CAS command handling this request (filled only if CAS support commands)
     * @param[in] cat_section CAT section to parse, null if tuner has disconnected
     * @param[in] tuner_num ID of tuner which received session
     * @return operation code: -1 if any error occured, 0 if CAT processing was scheduled properly
     */
    int (*pushCAT)(SvObject self_, QBCASCallbackCatFun *fun, void *arg, QBCASCmd *cmd_out, SvData catSection, int tuner_num);

    /**
     * Handles new EMM section
     * @param[in] self_ QBCAS instance
     * @param[in] fun callback to call when processing is done
     * @param[in] arg argument to pass in callback
     * @param[out] cmd_out CAS command handling this request (filled only if CAS support commands)
     * @param[in] pid PID on which this EMM was received
     * @param[in] emm_section EMM section to handle
     * @param[in] priority
     * @return operation code: -1 if any error occured, 0 if EMM processing was scheduled properly
     */
    int (*pushEMM)(SvObject self_, QBCASCallbackFun *fun, void *arg, QBCASCmd *cmd_out, int pid, SvData emmSection, bool priority);

    /**
     * Handles new ECM section
     * @param[in] self_ QBCAS instance
     * @param[in] sessionId id of session
     * @param[in] ecm_pid PID on which comes ECM section
     * @param[in] ecm_section ECM section
     * @param[in] position ecm position
     * @return ECM processing code (@see QBCASPushECMResponseCode)
     */
    int (*pushECM)(SvObject self_, int8_t sessionId, int ecm_pid, SvData ecmSection, SvTime position);

    /**
     * Manages descrambling on given pid
     * @param[in] self_ QBCAS instance
     * @param[in] sessionId id of session
     * @param[in] enable if descrambling should be enabled or disable
     * @param[in] pid channel pid on which descrambling process should be changed
     * @param[in] channel extra data about channel
     */
    int (*decryptPid)(SvObject self_, int8_t sessionId, bool enable, int pid, void *channel);

    /**
     * Pauses EMM processing for given timeout
     */
    int (*pauseEMMProcessing)(SvObject self_, SvTime timeout);

    bool (*hasCBCAtis)(SvObject self_);

    /**
     * Registers callbacks
     * @param self_ QBCAS instance
     * @param callbacks callback set to register
     * @param target pointer to caller
     * @param name name of callbacks set
     */
    void (*addCallbacks)(SvObject self_, const QBCASCallbacks callbacks, void *target, const char *name);

    /**
     * Unregisters callbacks
     */
    void (*removeCallbacks)(SvObject self_, const QBCASCallbacks callbacks, void *target);

    /**
     * Informs that standby action was starte or finished
     */
    int (*activeStandby)(SvObject self_, bool enable);

    /**
     * Returns info about CAS
     * @param[in] self_ QBCAS instance
     * @param[in] fun callback to be called when CAS info is completed
     * @param[in] arg argument to be passed in callback
     * @param[in] cmd_out
     */
    int (*getInfo)(SvObject self_, QBCASCallbackInfoFun *fun, void *arg, QBCASCmd *cmd_out);

    /**
     * Cancels given command
     * @param[in] self_ QBCAS instance
     * @param cmd command - can be NULL if commands are not supported by CAS implementation
     */
    void (*cancelCommand)(SvObject self_, QBCASCmd cmd);

    /**
     * Retrieves name of command
     * @param[in] self_ QBCAS instance
     * @param cmd command - can be NULL if commands are not supported by CAS implementation
     * @return comman name
     */
    const char *(*getCommandName)(SvObject self_, const QBCASCmd cmd);

    /**
     * Check CAS individualization state
     * @param[in] self_ QBCAS instance
     * @return individualization state
     */
    QBCASIndividualizationState (*getIndividualizationState)(SvObject self_);

    /**
     * Check CAS individualization state
     * @param[in] self_ QBCAS instance
     * @return individualization state
     */
    QBCASIndividualizationInfo (*performIndividualization)(SvObject self_);

    /**
     * Get custom PVR settings for implemented CAS
     * @param[in] self_ QBCAS instance
     * @param[out] settings @see QBCASPVRSettings_s
     */
    void (*getPVRSettings)(SvObject self_, struct QBCASPVRSettings_s *settings);
};

SvInterface QBCAS_getInterface(void);

/**
 * Returns used implementation of QBCAS interface
 *
 * @return instance of CAS
 */
SvObject QBCASGetInstance(void);

/**
 * Returns string representation of CAS smartcard state
 * @param state CAS smartcard state
 * @return name of state
 */
const char* QBCASSmartcardStateToString(QBCASSmartcardState state);

/**
 * Returns string representation of ECM status
 * @param status ECM status
 * @return name of ECM status
 */
const char* QBCASEcmStatusToString(QBCASEcmStatus status);

/**
 * Initializes CAS implementation
 * @returns operation code, 0 means success, other value means failure
 */
int QBCASInit(void);

/**
 * Deinitializes CAS implementation
 */
void QBCASDeinit(void);

/**
 * Creates instance of CAS
 *
 * @return instance of CAS
 */
extern SvObject QBCASCreateInstance(void);

#endif /* QBCAS_H_ */

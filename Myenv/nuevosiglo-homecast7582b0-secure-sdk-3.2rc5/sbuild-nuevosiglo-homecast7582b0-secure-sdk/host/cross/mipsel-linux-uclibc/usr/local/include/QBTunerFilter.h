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

#ifndef QB_TUNER_FILTER_H
#define QB_TUNER_FILTER_H

/**
 * @file QBTunerFilter.h Tuner filter library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvPlayerKit/SvChbuf.h>
#include <SvPlayerKit/SvMemCounter.h>
#include <dataformat/sv_data_format_type.h>
#include <QBTunerTypes.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTunerFilter QBTunerFilter: tuner filter library
 * @ingroup MediaPlayer
 * @{
 *
 * QBTunerFilter is an autonomous receiver of data from a single PID from a single tuner.
 * The PID to receive TS packets from can be specified in three ways:
 *  - explicitly,
 *  - as a program number (SID) to receive PMT,
 *  - by specifying a function that will be called to get PID when filter
 *    has connected to the desired mux.
 *
 * Tuner filter uses fibers to monitor tuner status, read and report data.
 * Fibers are always created in the main scheduler due to interaction
 * with underlying tuner engine that uses the same scheduler.
 **/


struct QBTunerFilter_s;
/**
 * QBTunerFilter class.
 * @note This is not an SvObject.
 */
typedef struct QBTunerFilter_s* QBTunerFilter;

/**
 * Create new filter on tuner @a tuner_num.
 *
 * @param[in] tuner_num  tuner index from which to read data (from sv_tuner API)
 * @param[in] name  descriptive name (for debugging purposes only)
 * @return  newly created tuner filter instance
 */
QBTunerFilter  QBTunerFilterCreate(int tuner_num, const char* name);

/**
 * Create new filter on tuner @a tuner_num.
 *
 * @param[in] tuner_num  tuner index from which to read data (from sv_tuner API)
 * @param[in] name  descriptive name (for debugging purposes only)
 * @param[in] max_callback_size  maximal data size that will be reported in a single callback
 * @returns  newly created tuner filter instance
 */
QBTunerFilter  QBTunerFilterCreateEx(int tuner_num, const char* name, int max_callback_size);

/**
 * Destroy tuner filter object.
 *
 * @note No more callbacks will be fired from the start of calling this function.
 *
 * @param[in] filter  filter object to be destroyed
 */
void QBTunerFilterDestroy(QBTunerFilter filter);

/**
 * Tuner filter application callback functions.
 *
 * Callbacks are always be called from an internal fiber, never from a QBTunerFilter API call.
 * All callbacks here are reentrant: during a callback, you can call any method on the QBTunerFilter
 * object, including destruction.
 */
struct QBTunerFilterCallbacks {
  /**
   * Notify that connection status has changed.
   *
   * This callback signals that data will start flowing (when @a is_connected is @c true)
   * or that data stream will stop. It is usually a good idea to flush all parsers in both
   * situations.
   *
   * @param[in] owner  opaque pointer to callbacks' owner
   * @param[in] filter  tuner filter handle
   * @param[in] is_connected  @c true if data can be received from tuner,
   *                          @c false if tuner is not tuned or is tuned to a mux incompatible with setup params
   */
  void (*connected) (void* owner, const struct QBTunerFilter_s* filter, bool is_connected);

  /**
   * Data chunk has been received.
   *
   * @param[in] owner  opaque pointer to callbacks' owner
   * @param[in] filter  tuner filter handle
   * @param[in] sb  received data packets, ownership of @a sb is passed to the callee
   */
  void (*data)      (void* owner, const struct QBTunerFilter_s* filter, SvBuf sb);
};

/**
 * Set tuner filter callbacks.
 *
 * @param[in] filter  filter object
 * @param[in] callbacks  callbacks
 * @param[in] owner  opaque pointer to @a callbacks' owner
 */
void QBTunerFilterSetCallbacks(QBTunerFilter filter, const struct QBTunerFilterCallbacks* callbacks, void* owner);

/**
 * Function for selecting which PID is to be received after connecting to a new mux.
 *
 * If QBTunerFilterSetupParams::programNumber attribute is valid, then normally filter receives PMT from that program,
 * but when QBTunerFilterSetupParams::selectFun attribute is given, the filter asks which PID to receive upon getting
 * first PMT from that program.
 *
 * If program number is not specified, then this function is called when the filter connects to the mux, before
 * QBTunerFilterCallbacks::connected() is called. In such case @a format is @c NULL.
 *
 * @param[in] owner  opaque pointer to callbacks' owner
 * @param[in] filter  tuner filter handle
 * @param[in] format  description of the program received from PMT or @c NULL
 */
typedef int (QBTunerFilterSelectPidFun)(void* owner, const struct QBTunerFilter_s* filter, const struct svdataformat* format);

/**
 * Tuner filter setup parameters.
 */
struct QBTunerFilterSetupParams {
  /// use this mux, if possible (otherwise filter will report not connected and not give any data); ignored if @a anyFreq is @c true
  struct QBTunerMuxId mux_id;
  /// @c true to allow filter to use any frequency that the tuner will be tuned to, now and in the future
  bool anyFreq;

  /// @deprecated
  bool canChangeFreq;

  /// receive this PID (use negative value for unknown)
  int pid;
  /// function to select which PID to receive (used only if @a pid is negative), called after connecting to a mux
  QBTunerFilterSelectPidFun* selectFun;
  /**
   * if >= 0: receives PMT from this service (but when used with @a selectFun, other PID can be chosen, and filter
   * will wait until a receiver of this service exists to get the PMT from this service and use @a selectFun)
   **/
  int programNumber;

  /// buffer size for internal receiver (use negative value to request small buffer of default size)
  int bufferSize;

  /// frequency of polling tuner buffer, in milliseconds (use value <= for default frequency)
  int updateDelayMs;

  // more?
};

/**
 * Start monitoring tuner and receiving data according to @a params.
 *
 * Data will come with callbacks, set with QBTunerFilterSetCallbacks().
 * Monitoring/receiving can be cancelled with QBTunerFilterStop().
 *
 * @param[in] filter   tuner filter handle
 * @param[in] params   parameters describing connection and receiving setup
 */
void QBTunerFilterSetup(QBTunerFilter filter, const struct QBTunerFilterSetupParams* params);

/**
 * Stop processing that was started with QBTunerFilterSetup().
 *
 * @param[in] filter   tuner filter handle
 */
void QBTunerFilterStop(QBTunerFilter filter);

/**
 * Setup how data buffers are allocated, and have to be treated by callbacks.
 *
 * If your callback performs its data processing immediately, you can enable in-place
 * data reporting by calling this method with @a inplace set to @c true. This will have
 * positive impact on performance.
 *
 * @param[in] filter  tuner filter handle
 * @param[in] inplace  if @c true, buffers passed to callbacks are re-used, and will no longer be valid after callback returns, should not be released, etc;
 *                     if @c false (default), the buffers are allocated every time, and passed to callbacks with ownership transfer
 */
void QBTunerFilterReportDataInplace(struct QBTunerFilter_s* filter, bool inplace);


/**
 * Get current PID that the filter is currently receiving.
 *
 * @param[in] filter  tuner filter handle
 * @return  PID that @a filter is currenlty receiving, or @c -1 if not receving any PID
 */
int QBTunerFilterGetPid(const struct QBTunerFilter_s* filter);

/** Get current mux that the filter's tuner is currently tuned to.
 *
 * @param[in] filter  tuner filter handle
 * @return  current mux that filter's tuner is tuned to (not necessarily compatible with filter's settings)
 */
struct QBTunerMuxId QBTunerFilterGetMuxId(const struct QBTunerFilter_s* filter);

/**
 * Get current data format (valid only when receiving in program mode.)
 *
 * @param[in] filter  tuner filter handle
 * @return  data format (PMT) from QBTunerFilterSetupParams::programNumber, @c NULL if unknown
 */
struct svdataformat* QBTunerFilterGetFormat(const struct QBTunerFilter_s* filter);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_TUNER_FILTER_H

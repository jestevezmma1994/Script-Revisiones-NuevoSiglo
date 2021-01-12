/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TS_PES_PARSER_H
#define QB_TS_PES_PARSER_H

/**
 * @file QBTSPesParser.h QBTSPESParser API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

// TODO: ingroup
/**
 * @defgroup QBTSParser QBTSParser classes
 * @{
 *
 * QBTSSectionParser represents stateful PES parser for a single PID.
 * It is given an array of TS packets, and it processes them synchronously, returning results with given callbacks.
 */

#include <SvPlayerKit/SvBuf.h>
#include <SvPlayerKit/SvChbuf.h>
#include <SvPlayerKit/SvMemCounter.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBTSPesParser_s;
/**
 * QBTSPesParser class.
 * @class QBTSPesParser
 */
typedef struct QBTSPesParser_s  QBTSPesParser;

/**
 * QBTSPesParserCallbacks_s class.
 * @class QBTSPesParserCallbacks_s
 * Only one of @a data and @a data2 callbacks should normally be provided.
 * If both are given, @a data will be used for bounded PES.
 * For unbounded PES only @a data2 will be used (if @a data2 is not given, unbounded PES will be discarded).
 */
struct QBTSPesParserCallbacks_s
{
  // unused
  void (*error) (void* target, QBTSPesParser* parser, int err);
  // unused
  void (*disc)  (void* target, QBTSPesParser* parser);
  /**
   * A new PES was parsed and is returned.
   * This callback can only be used for bounded PES.
   * @param[in] sb  buffer with the PES data, callee gains the ownership of this buffer
   */
  void (*data)  (void* target, QBTSPesParser* parser, SvBuf sb);
  /**
   * A new PES was parsed and is returned.
   * Unbounded PES can only be returned with this callback, since they have more complicated memory management.
   * @param[in] chb  buffer with chunks of PES data, callee can transfer all chunks from @a chb and thus gain ownership of them
   */
  void (*data2) (void* target, QBTSPesParser* parser, SvChbuf chb);
};

/**
 *  Allocate a new PES parser.
 *  @returns newly allocated PES parser, it is a generic object, and so must be released after it's no longer needed
 */
QBTSPesParser* QBTSPesParserCreate(void);

/**
 *  Pass mem-counter to be used when allocating all PES buffers.
 *  @param[in] self PES parser
 *  @param[in] memCounter mem-counter to be used with allocated buffers
 */
void QBTSPesParserSetMemCounter(QBTSPesParser* self, SvMemCounter memCounter);

/**
 *  Pass callbacks to be called by the parser as the PES are being found in the stream.
 *  @param[in] self PES parser
 *  @param[in] callbacks callbacks to be called
 *  @param[in] target parameter to the callbacks
 */
void QBTSPesParserSetCallbacks(QBTSPesParser* self, const struct QBTSPesParserCallbacks_s* callbacks, void* target);

/**
 *  Launch parsing on provided packets.
 *  This function processes the packets synchronously, calling the callback functions in the meantime.
 *  It is assumed, that the buffer only contains packets from one PID, since no PID filtering is being done.
 *  @param[in] self  PES parser
 *  @param[in] packets  buffer with TS packets (at least @a packetCnt), must be 4-byte aligned
 *  @param[in] packetCnt  amount of packets passed in @a packets buffer
 */
void QBTSPesParserRun(QBTSPesParser* self, const unsigned char* packets, int packetCnt);

/**
 *  Launch parsing on provided packets, only using packets from given PID value.
 *  This function processes the packets synchronously, calling the callback functions in the meantime.
 *  Packets are filtered with given @a refPid before parsing.
 *  @param[in] self  PES parser
 *  @param[in] packets  buffer with TS packets (at least @a packetCnt), must be 4-byte aligned
 *  @param[in] packetCnt  amount of packets passed in @a packets buffer
 *  @param[in] refPid  PID value, for using only packets with this PID
 */
void QBTSPesParserRunAndFilter(QBTSPesParser* self, const unsigned char* packets, int packetCnt, int refPid);

/**
 *  Reset parser's internal state.
 *  @param[in] self  PES parser
 */
void QBTSPesParserClear(QBTSPesParser* self);

#ifdef __cplusplus
}
#endif

/**
 * @}
 **/

#endif // #ifndef QB_TS_PES_PARSER_H

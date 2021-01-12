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

#ifndef QB_TS_SECTION_PARSER_H
#define QB_TS_SECTION_PARSER_H

/**
 * @file QBTSSectionParser.h QBTSSectionParser API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

// TODO: ingroup
/**
 * @defgroup QBTSParser QBTSParser classes
 * @{
 *
 * QBTSSectionParser represents stateful section parser for a single PID.
 * It is given an array of TS packets, and it processes them synchronously, returning results with given callbacks.
 */

#include "QBTSSectionFilter.h"
#include "QBTSSectionCustomFilter.h"

#include <SvPlayerKit/SvBuf.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBTSSectionParser_s;
/**
 * QBTSSectionParser class.
 * @class QBTSSectionParser
 */
typedef struct QBTSSectionParser_s          QBTSSectionParser;

/**
 * @brief QBTSSectionParser error codes.
 */
typedef enum {
    QBTSSectionParserError_unknown,     /**< unknown */
    QBTSSectionParserError_wrongCrc     /**< wrong section's CRC */
} QBTSSectionParserError;

/**
 * QBTSSectionParserCallbacks_s class.
 * @class QBTSSectionParserCallbacks_s
 */
struct QBTSSectionParserCallbacks_s
{
  /**
   * Error during section parsing.
   *
   * @param[in] target  listener
   * @param[in] parser  section parser
   * @param[in] err     error code
   */
  void (*error) (void* target, QBTSSectionParser* parser, QBTSSectionParserError err);

  // unused
  void (*disc)  (void* target, QBTSSectionParser* parser);
  /**
   * A new section was parsed and is returned.
   * @param[in] sb  buffer with the section data, callee gains the ownership of this buffer (unless parser is in 'inplace' mode)
   */
  void (*data)  (void* target, QBTSSectionParser* parser, SvBuf sb);
};

/**
 *  Allocate a new section parser.
 *  @returns newly allocated section parser, it is a generic object, and so must be released after it's no longer needed
 */
QBTSSectionParser* QBTSSectionParserCreate(void);

/**
 * Enable or disable section's CRC checking.
 * Error callback is invoked in case of wrong CRC.
 *
 * @param[in] self      section parser
 * @param[in] enable    @c true if CRC checking should be enabled, @c false otherwise
 */
void QBTSSectionParserSetupCrcChecking(QBTSSectionParser* self, bool enable);

/**
 *  Pass callbacks to be called by the parser as the sections are being found in the stream.
 *  @param[in] self section parser
 *  @param[in] callbacks callbacks to be called
 *  @param[in] target parameter to the callbacks
 */
void QBTSSectionParserSetCallbacks(QBTSSectionParser* self, const struct QBTSSectionParserCallbacks_s* callbacks, void* target);

/**
 *  Instruct the parser to return section data in a statically allocated buffer, returning it in 'data' callback without passing buffer ownership.
 *  In the 'data' callback, callee must handle the section synchronously, as the buffer will no longer be valid after the function returns.
 *  @param[in] self  section parser
 *  @param[in] maxSize  max section size that will be accepted by the parser (all above that will be discarded)
 */
void QBTSSectionParserReportDataInplace(QBTSSectionParser* self, int maxSize);

/**
 *  Add standard section filter (data + mask) to the section parser.
 *  All added filters must pass for a section, or it will be rejected.
 *  @param[in] self  section parser
 *  @param[in] filter  filter to be added
 */
void QBTSSectionParserAddFilter(QBTSSectionParser* self, QBTSSectionFilter* filter);

/**
 *  Add custom section filter (with virtual function) to the section parser.
 *  All added filters must pass for a section, or it will be rejected.
 *  @param[in] parser  section parser
 *  @param[in] filter  filter to be added
 */
void QBTSSectionParserAddCustomFilter(QBTSSectionParser* parser, QBTSSectionCustomFilter* filter);

/**
 *  Launch parsing on provided packets.
 *  This function processes the packets synchronously, calling the callback functions in the meantime.
 *  It is assumed, that the buffer only contains packets from one PID, since no PID filtering is being done.
 *  @param[in] self  section parser
 *  @param[in] packets  buffer with TS packets (at least @a packetCnt), must be 4-byte aligned
 *  @param[in] packetCnt  amount of packets passed in @a packets buffer
 */
void QBTSSectionParserRun(QBTSSectionParser* self, const unsigned char* packets, int packetCnt);

/**
 *  Launch parsing on provided packets, only using packets from given PID value.
 *  This function processes the packets synchronously, calling the callback functions in the meantime.
 *  Packets are filtered with given @a refPid before parsing.
 *  @param[in] self  section parser
 *  @param[in] packets  buffer with TS packets (at least @a packetCnt), must be 4-byte aligned
 *  @param[in] packetCnt  amount of packets passed in @a packets buffer
 *  @param[in] refPid  PID value, for using only packets with this PID
 */
void QBTSSectionParserRunAndFilter(QBTSSectionParser* self, const unsigned char* packets, int packetCnt, int refPid);

/**
 *  Reset parser's internal state.
 *  @param[in] self  section parser
 */
void QBTSSectionParserClear(QBTSSectionParser* self);

#ifdef __cplusplus
}
#endif

/**
 * @}
 **/

#endif // #ifndef QB_TS_SECTION_PARSER_H

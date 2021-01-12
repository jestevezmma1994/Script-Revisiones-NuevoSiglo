/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_M3U_PARSER_H
#define QB_M3U_PARSER_H

/**
 * @file QBM3uParser.h
 * @brief Parser of the m3u and m3u8 files.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvPlayerKit/SvChbuf.h>

#include <stdbool.h>

/**
 * @defgroup QBM3uParser M3U and M3U8 files parser
 * @{
 *
 * M3U and M3U8 files parser
 **/

/**
 * M3u track info.
 */
struct QBM3uItem_s {
    /// super class
    struct SvObject_ super_;
    /// track name
    SvString name;
    /// track lenght
    double trackLength;
    /// track url
    SvString track;
};
typedef struct QBM3uItem_s* QBM3uItem;

/**
 * Type of parser handle.
 */
typedef struct QBM3uParser_s* QBM3uParser;

/**
 * Callback function pointer for reporting track info.
 *
 * @param[in] target    callback's owner handle
 * @param[in] parser    parser handle
 * @param[in] item      track info/item
 */
typedef void QBM3uParserGotItemFun(void* target, QBM3uParser parser, QBM3uItem item);

/**
 * Callback function pointer for reporting attribute and value.
 *
 * @param[in] target    callback's owner handle
 * @param[in] parser    parser handle
 * @param[in] name      attribute name
 * @param[in] value     attribute value
 */
typedef void QBM3uParserGotAttributeFun(void* target, QBM3uParser parser, SvString name, SvString value);

/**
 * Callback function pointer for reorting parsing errors.
 *
 * @param[in] target    callback's owner handle
 * @param[in] parser    parser handle
 */
typedef void QBM3uParserErrorFun(void* target, QBM3uParser parser);

/**
 * Create parser of the m3u and m3u8 files.
 *
 * @param[in] maxLineLength     maximum acceptable line length
 * @param[in] itemFun           callback for reporting QBM3uItem objects
 * @param[in] attrFun           callback for reporting attributes
 * @param[in] errorFun          callback for reporting parsing errors
 * @param[in] target            pointer to the callbacks owner
 * @return                      Handle of QBM3uParser parser or @c NULL on error
 */
QBM3uParser QBM3uParserCreate(int maxLineLength,
                              QBM3uParserGotItemFun* itemFun,
                              QBM3uParserGotAttributeFun* attrFun,
                              QBM3uParserErrorFun* errorFun,
                              void *target);

/**
 * Create m3u playlist parser and push file content to it.
 *
 * Parser is set EOF after creation. No additional QBM3uParserPush calls are allowed.
 *
 * @param[in] filePath      Handle of m3u playlist path
 * @param[in] maxLineLength Maximal length of single line in playlist file
 * @param[in] maxFileSize   Maximal size of playlist file
 * @param[in] itemFun       New playlist item found callback
 * @param[in] attrFun       New playlist attribute found callback
 * @param[in] errorFun      Playlist error callback
 * @param[in] target        Callback's userdata
 * @return                  Handle of QBM3uParser parser or @c NULL on error
 */
QBM3uParser QBM3uParserCreateWithFile(SvString filePath,
                                      int maxLineLength,
                                      off_t maxFileSize,
                                      QBM3uParserGotItemFun* itemFun,
                                      QBM3uParserGotAttributeFun* attrFun,
                                      QBM3uParserErrorFun* errorFun,
                                      void *target);

/**
 * Clear parser state and queue.
 *
 * @param[in] self  m3u parser handle
 */
void QBM3uParserClear(QBM3uParser self);

/**
 * Push data chunk to be parsed.
 *
 * @param[in] self  m3u parser handle
 * @param[in] buf   data chunk to be parsed
 * @return          @c true if the data were pushed on queue or @c false if the state was invalid
 */
bool QBM3uParserPush(QBM3uParser self, SvBuf buf);

/**
 * Notify parser that the EOF was reached.
 *
 * @param[in] self  m3u parser handle
 * @return          @c true if the state was changed, @c false if the state was invalid
 */
bool QBM3uParserSetEof(QBM3uParser self);

/**
 * Run parser in the blocking mode.
 *
 * @param[in] self  m3u parser handle
 * @return          @c true if the data were parsed, @c false if the parser is in error state
 */
bool QBM3uParserRun(QBM3uParser self);

/**
 * Schedule parsing.
 *
 * @param[in] self  m3u parser handle
 * @return          @c true if the run was scheduled, @c false if the parser is in error state
 */
bool QBM3uParserScheduledRun(QBM3uParser self);

/**
 * Stop parsing.
 *
 * @param[in] self  m3u parser handle
 */
void QBM3uParserStop(QBM3uParser self);

/**
 * Get last parsed line length.
 *
 * @param[in] self  m3u parser handle
 * @return          last line length or -1 in case of parsing error
 */
int QBM3uParserGetLineLength(QBM3uParser self);

/**
 * @}
 **/

#endif // #ifndef QB_M3U_PARSER_H

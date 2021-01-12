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

#ifndef QBMPDPARSER_H_
#define QBMPDPARSER_H_

/**
 * @file QBMPDParser.h
 * @brief MPEG-DASH Media Presentation Description parser library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvPlayerKit/SvBuf.h>

/**
 * @defgroup QBMPDParser Media Presentation Description parser class
 * @{
 **/

/**
 * Media Presentation Description parser class.
 * @class QBMPDParser
 * @extends SvObject
 **/
typedef struct QBMPDParser_ *QBMPDParser;

/**
 * Get runtime type identification object representing
 * type of MPD parser class.
 *
 * @return MPD parser class
 **/
extern SvType
QBMPDParser_getType(void);

/**
 * Create Media Presentation Description manifest parser.
 *
 * @param[out] errorOut error info
 * @return              new instance of MPD manifest parser, @c NULL in case of error
 **/
extern QBMPDParser
QBMPDParserCreate(SvErrorInfo *errorOut);

/**
 * Parse next chunk of XML data of Media Presentation Description manifest.
 *
 * @param[in]  self     MPD parser handle
 * @param[in]  buf      XML representation of MPD manifest
 * @param[out] errorOut error info
 **/
extern void
QBMPDParserParseDataChunk(QBMPDParser self,
                          SvBuf buf,
                          SvErrorInfo *errorOut);

/**
 * Finish parsing of Media Presentation Description manifest.
 *
 * @param[in]  self     MPD parser handle
 * @param[out] errorOut error info
 **/
extern void
QBMPDParserDataFinished(QBMPDParser self,
                        SvErrorInfo *errorOut);

/**
 * Reset Media Presentation Description manifest parser.
 *
 * @param[in]  self     MPD parser handle
 * @param[out] errorOut error info
 **/
extern void
QBMPDParserReset(QBMPDParser self,
                 SvErrorInfo *errorOut);

/**
 * Set Media Presentation Description manifest parser listener.
 *
 * Listener must implement @ref QBIPStreamManifestParserListener
 *
 * @param[in]  self     MPD parser handle
 * @param[in]  listener MPD parser listener handle
 * @param[out] errorOut error info
 **/
extern void
QBMPDParserSetListener(QBMPDParser self,
                       SvObject listener,
                       SvErrorInfo *errorOut);

/**
 * Check weather parser already handled data element.
 *
 * @param[in]  self     MPD parser handle
 * @return              @c true if handled, otherwise @c false
 **/
extern bool
QBMPDParserHandledElement(QBMPDParser self);

/**
 * @}
 **/

#endif /* QBMPDPARSER_H_ */

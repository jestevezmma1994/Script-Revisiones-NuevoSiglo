/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUBIWAREMWNPVRRESPONSEPARSER_H_
#define CUBIWAREMWNPVRRESPONSEPARSER_H_

#include <CubiwareMWClient/NPvr/CubiwareMWNPvrRequest.h>
#include <SvEPGDataLayer/SvEPGChannelList.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvDataBucket2/SvDataBucket.h>
#include <SvDataBucket2/SvDBRawObject.h>

#include <stdbool.h>

/**
 * @defgroup CubiwareMWNPvrResponseParser Cubiware MW NPVr response handling functions
 * @ingroup CubiwareMWClient
 **/

/**
 * Parsing results.
 */
struct CubiwareMWNPvrParserResultData_s {
    struct SvObject_ super_;

    SvDBObject dbObject;
    SvObject parsedObject;
    bool changed;
};
typedef struct CubiwareMWNPvrParserResultData_s* CubiwareMWNPvrParserResultData;

/**
 * Parsing finished callback type.
 *
 * @param[in] listener          listener self object
 * @param[in] parsingResults    array of parsed objects
 */
typedef void (*CubiwareMWNPvrResponseParserFinished)(SvObject listener, SvArray parsingResults);

/**
 * @brief CubiwareMWNPvrResponseParser
 *
 * Parser of NPvr calls responses.
 */
typedef struct CubiwareMWNPvrResponseParser_s *CubiwareMWNPvrResponseParser;

/**
 * Get runtime type identification object
 * representing CubiwareMWNPvrResponseParser class.
 *
 * @return CubiwareMWNPvrResponseParser provider class
 **/
SvType
CubiwareMWNPvrResponseParser_getType(void);

/**
 * Create CubiwareMWNPvrResponseParser.
 *
 * @param[in] storage               SvDataBucket to store NPvr related objects
 * @param[in] channelList           channel list used to fill some data in the parsed objects
 * @param[in] listener              parser listener
 * @param[in] finishCallback        parsing finished callback function
 * @return response parser object or NULL in case of wrong params
 */
CubiwareMWNPvrResponseParser
CubiwareMWNPvrResponseParserCreate(SvDataBucket storage, SvEPGChannelList channelList, SvObject listener, CubiwareMWNPvrResponseParserFinished finishCallback);

/**
 * Start parsing.
 *
 * @param[in] self               parser object
 * @param[in] type               type of the request
 * @param[in] productsToParse    array of objects to parse
 * @return  0 if OK, -1 on error
 */
int
CubiwareMWNPvrResponseParserStart(CubiwareMWNPvrResponseParser self, CubiwareMWNPvrRequestType type, SvArray productsToParse);

/**
 * Stop parsing and release resources.
 *
 * @param[in] self      parser object
 * @return  0 if OK, -1 on error
 */
int
CubiwareMWNPvrResponseParserStop(CubiwareMWNPvrResponseParser self);

/** @} */

#endif /* CUBIWAREMWNPVRRESPONSEPARSER_H_ */

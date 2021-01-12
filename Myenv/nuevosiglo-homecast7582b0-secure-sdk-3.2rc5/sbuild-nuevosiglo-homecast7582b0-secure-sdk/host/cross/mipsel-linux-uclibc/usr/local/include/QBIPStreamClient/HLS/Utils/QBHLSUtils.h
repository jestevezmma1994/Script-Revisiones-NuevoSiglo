/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBIPSTREAMCLIENT_QBHLSUTILS_H_
#define QBIPSTREAMCLIENT_QBHLSUTILS_H_

/**
 * @file QBHLSUtils.h Utilities used in HLS implementation.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/HLS/M3U8/QBM3U8Rendition.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>

/**
 * @defgroup QBHLSUtils Hypertext Live Streaming Utility functions
 * @ingroup QBIPStreamClient
 * @{
 **/

/**
 * Parse an attribute list to a HashTable.
 *
 * @param[in] attributeList attribute list string
 * An attribute list is defined as comma-sparated list of attribute/value
 * pairs with no whitespace.
 *
 * An attribute/value pair has the following syntax:
 * AttributeName=AtrributeValue
 *
 * An AttributeName is an unquoted string containing characters from the set
 * [A..Z], [0..9] and '-'. Therefore, AttributeNames contain only uppercase
 * letters, not lowercase. There MUST NOT be any whitespace  between the
 * AttributeName and the '=' character, nor between the '=' character and the
 * AttributeValue.
 *
 * Detailed description of allowed AttributeValues forms can be found here:
 * https://tools.ietf.org/html/draft-pantos-http-live-streaming#section-4.2
 *
 * Examples of attribute lists (each line contains a unique attribue list):
 * BANDWIDTH=1280000,AVERAGE-BANDWIDTH=1000000
 * TYPE=VIDEO,GROUP-ID="mid",NAME="Centerfield",DEFAULT=NO,URI="mid/centerfield/audio-video.m3u8"
 * METHOD=AES-128,URI="https://priv.example.com/key.php?r=53"
 * DATA-ID="com.example.title",LANGUAGE="sp",VALUE="Este es un ejemplo"
 *
 * @return new hash table with parsed attribute list
 **/
extern SvHashTable
QBHLSUtilsParseAttributeList(SvString attributeList);

/**
 * Create QBM3U8RenditionType value from a cstring.
 *
 * @param[in]  cstring input string
 * @return     QBM3U8RenditionType value coresponding to cstring
 **/
extern QBM3U8RenditionType
QBHLSUtilsParseRenditionType(const char const *cstring);

/**
 * @}
 **/

#endif /* QBIPSTREAMCLIENT_QBHLSUTILS_H_ */

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

#ifndef QBMPDPROGRAMINFORMATION_H_
#define QBMPDPROGRAMINFORMATION_H_

/**
 * @file QBMPDProgramInformation.h
 * @brief Program Information element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>

/**
 * @defgroup QBMPDProgramInformation Program Information element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Program Information element class.
 * @class QBMPDProgramInformation
 * @extends SvObject
 **/
typedef struct QBMPDProgramInformation_ *QBMPDProgramInformation;

/**
 * Get runtime type identification object representing
 * type of Program Information element class.
 *
 * @return Program Information element class
 **/
extern SvType
QBMPDProgramInformation_getType(void);

/**
 * Get information about language of Program Information element.
 *
 * @param[in] self Program Information element handle
 * @return         language code, @c NULL in case of error
 **/
extern SvString
QBMPDProgramInformationGetLang(QBMPDProgramInformation self);

/**
 * Get URL providing more informations about Media Presentation of Program Information element.
 *
 * @param[in] self Program Information element handle
 * @return         URL providing more informations about Media Presentation (absolute URL), @c NULL in case of error
 **/
extern SvURI
QBMPDProgramInformationGetMoreInformationURL(QBMPDProgramInformation self);

/**
 * Get title of Program Information element.
 *
 * @param[in] self Program Information element handle.
 * @return         title of Media Presentation, @c NULL in case of error
 **/
extern SvString
QBMPDProgramInformationGetTitle(QBMPDProgramInformation self);

/**
 * Get source information of Program Information element.
 *
 * @param[in] self Program Information element handle
 * @return         information about source of Media Presentation, @c NULL in case of error
 **/
extern SvString
QBMPDProgramInformationGetSource(QBMPDProgramInformation self);

/**
 * Get copyright information of Program Information element.
 *
 * @param[in] self Program Information element handle
 * @return         copyright information of Media Presentation, @c NULL in case of error
 **/
extern SvString
QBMPDProgramInformationGetCopyright(QBMPDProgramInformation self);

/**
 * @}
 **/

#endif /* QBMPDPROGRAMINFORMATION_H_ */

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

#ifndef QBMPDPROGRAMINFORMATIONINTERNAL_H_
#define QBMPDPROGRAMINFORMATIONINTERNAL_H_

/**
 * @file QBMPDProgramInformationInternal.h
 * @brief Program Information element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDProgramInformation.h>

/**
 * @addtogroup QBMPDProgramInformation
 * @{
 **/

/**
 * Create Program Information element object.
 *
 * @param[out] errorOut error info
 * @return              new instance of Program Information element, @c NULL in case of error
 **/
extern QBMPDProgramInformation
QBMPDProgramInformationCreate(SvErrorInfo *errorOut);

/**
 * Set language for Program Information element.
 *
 * @param[in]  self     Program Information element handle
 * @param[in]  lang     language code
 * @param[out] errorOut error info
 **/
extern void
QBMPDProgramInformationSetLang(QBMPDProgramInformation self,
                               SvString lang,
                               SvErrorInfo *errorOut);

/**
 * Set URL providing more informations about Media Presentation for Program Information element.
 *
 * @param[in]  self               Program Information element handle
 * @param[in]  moreInformationURL URL providing more informations about Media Presentation (absolute URL)
 * @param[out] errorOut           error info
 **/
extern void
QBMPDProgramInformationSetMoreInformationURL(QBMPDProgramInformation self,
                                             SvURI moreInformationURL,
                                             SvErrorInfo *errorOut);

/**
 * Set tile for Program Information element.
 *
 * @param[in]  self     Program Information element handle
 * @param[in]  title    title for Media Presentation
 * @param[out] errorOut error info
 **/
extern void
QBMPDProgramInformationSetTitle(QBMPDProgramInformation self,
                                SvString title,
                                SvErrorInfo *errorOut);

/**
 * Set source information for Program Information element.
 *
 * @param[in]  self     Program Information element handle
 * @param[in]  source   information about orginal source of Media Presentation
 * @param[out] errorOut error info
 **/
extern void
QBMPDProgramInformationSetSource(QBMPDProgramInformation self,
                                 SvString source,
                                 SvErrorInfo *errorOut);

/**
 * Set copyright information for Program Information element.
 *
 * @param[in]  self      Program Information element handle
 * @param[in]  copyright information about copyright statement for Media Presentation
 * @param[out] errorOut  error info
 **/
extern void
QBMPDProgramInformationSetCopyright(QBMPDProgramInformation self,
                                    SvString copyright,
                                    SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDPROGRAMINFORMATIONINTERNAL_H_ */

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

#ifndef DATE_TIME_LOGIC_H_
#define DATE_TIME_LOGIC_H_

#include <SvFoundation/SvObject.h>
#include <mpeg_tables/tdt.h>
#include <mpeg_tables/tot.h>
#include <main_decl.h>
#include <SvFoundation/SvArray.h>
#include <SvCore/SvErrorInfo.h>

/**
* @file DateTimeLogic.h DateTimeLogic class API
* @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
* @endxmlonly
**/

/**
 * @defgroup DateTimeLogic Date Time Logic class
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * DateTimeLogic class.
 * @class DateTimeLogic
 **/
typedef struct DateTimeLogic_ *DateTimeLogic;

/**
  * Create DateTimeLogic instance.
  * If you wish to use dvb time and date from mpeg tables register suitable callbacks.
  *
  * @param[in] appGlobals   appGlobals handle
  * @return                 date time logic object
  **/
extern DateTimeLogic DateTimeLogicCreate(AppGlobals appGlobals) __attribute__((weak));

/**
 * Add an array of allowed muxes for DateTimeLogic.
 *
 * @param[in] self      DateTimeLogic handle
 * @param[in] muxes     array of allowed muxes of type QBTunerMuxIdObj*
 * @param[out] errorOut error info handle
 **/
void DateTimeLogicSetAllowedMuxes(DateTimeLogic self, SvArray muxes, SvErrorInfo* errorOut) __attribute__((weak));

/**
 * @}
 **/

#endif

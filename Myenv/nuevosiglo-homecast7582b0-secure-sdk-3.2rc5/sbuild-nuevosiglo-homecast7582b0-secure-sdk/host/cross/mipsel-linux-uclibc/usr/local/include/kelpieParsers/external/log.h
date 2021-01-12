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

#ifndef KPPARSERS_EXTERNAL_LOG_H
#define KPPARSERS_EXTERNAL_LOG_H

#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>

#ifdef __cplusplus
extern "C" {
#endif

#define kplog_error(fmt, ...)   do { SvLogError(  COLBEG() fmt COLEND_COL(red),    ##__VA_ARGS__); } while(0)
#define kplog_warn(  fmt, ...)  do { SvLogWarning(COLBEG() fmt COLEND_COL(yellow), ##__VA_ARGS__); } while(0)

SV_DECL_INT_ENV_FUN_DEFAULT( env_kplog_level, 1, "kpParsersLogLevel", "" );

#define kplog_debug(prio, fmt, ...)  do { if (env_kplog_level() >= (prio)) { SvLogNotice(fmt, ##__VA_ARGS__); } } while(0)

#ifdef __cplusplus
}
#endif

#endif // #ifndef KPPARSERS_EXTERNAL_LOG_H

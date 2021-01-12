/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SMOOTH_STREAMING_LOG_H
#define SMOOTH_STREAMING_LOG_H

#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>

#ifndef SS_CUSTOM_LOG_PREFIX
#define SS_CUSTOM_LOG_PREFIX "cont_parser_smooth"
#endif


#if SV_LOG_LEVEL > 0

SV_DECL_INT_ENV_FUN_DEFAULT(log_level, 4, "QBSSLogLevel", "log level");
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_trickplay, 0, "QBSSLogTrickplay", "");

#define log_func()              if (log_level() >= 6) { SvLogNotice(SS_CUSTOM_LOG_PREFIX " :: %s ", __func__); }
#define log_debug(fmt, ...)     if (log_level() >= 5) { SvLogNotice(COLBEG() SS_CUSTOM_LOG_PREFIX " :: %s " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); }
#define log_notice(fmt, ...)    if (log_level() >= 4) { SvLogNotice(COLBEG() SS_CUSTOM_LOG_PREFIX " :: " fmt COLEND_COL(green), ## __VA_ARGS__); }
#define log_info(fmt, ...)      if (log_level() >= 3) { SvLogNotice(COLBEG() SS_CUSTOM_LOG_PREFIX " :: " fmt COLEND_COL(green), ## __VA_ARGS__); }
#define log_warn(fmt, ...)      if (log_level() >= 2) { SvLogWarning(SS_CUSTOM_LOG_PREFIX " %s %d " fmt, __func__, __LINE__, ## __VA_ARGS__); }
#define log_error(fmt, ...)     if (log_level() >= 1) { SvLogError(SS_CUSTOM_LOG_PREFIX " %s %d " fmt, __func__, __LINE__, ## __VA_ARGS__); }

#define log_trick(fmt, ...)     if (env_log_trickplay() > 0) { SvLogNotice(COLBEG() SS_CUSTOM_LOG_PREFIX " ::%s::%d::" fmt COLEND_COL(cyan), __func__, __LINE__, ## __VA_ARGS__); }

#else

#define log_func(fmt, ...)
#define log_debug(fmt, ...)
#define log_notice(fmt, ...)
#define log_info(fmt, ...)
#define log_warn(fmt, ...)
#define log_error(fmt, ...)
#define log_trick(fmt, ...)

#endif

#endif // SMOOTH_STREAMING_LOG_H

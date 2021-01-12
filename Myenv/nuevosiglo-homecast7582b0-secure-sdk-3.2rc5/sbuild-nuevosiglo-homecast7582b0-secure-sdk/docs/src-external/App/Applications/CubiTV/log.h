/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef LOG_H_
#define LOG_H_

#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>

#if SV_LOG_LEVEL > 0
    SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, "QBCall2ActionLogLevel", "QBCall2Action log level");
    #define log_fun(fmt, ...)   do { if(env_log_level() >= 6) { SvLogNotice(COLBEG() "module :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
    #define log_debug(fmt, ...) do { if (env_log_level() >= 1) { SvLogNotice(COLBEG() "module :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
    #define log_deep(fmt, ...)  do { if (env_log_level() >= 4) { SvLogNotice(COLBEG() "module :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } } while (0)
    #define log_info(fmt, ...)  do { if (env_log_level() >= 3) { SvLogNotice(COLBEG() "module :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } } while (0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() "module :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } } while (0)
    #define log_error(fmt, ...) do { if (env_log_level() >= 1) { SvLogError(COLBEG() "module :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } } while (0)
#else
    #define log_fun(fmt, ...)
    #define log_debug(fmt, ...)
    #define log_deep(fmt, ...)
    #define log_info(fmt, ...)
    #define log_state(fmt, ...)
    #define log_error(fmt, ...)
#endif

#endif /* LOG_H_ */

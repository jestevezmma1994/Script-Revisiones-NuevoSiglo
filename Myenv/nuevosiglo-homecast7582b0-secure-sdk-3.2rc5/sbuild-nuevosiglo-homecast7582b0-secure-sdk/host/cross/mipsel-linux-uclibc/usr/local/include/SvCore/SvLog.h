/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_LOG_H_
#define SV_LOG_H_

/**
 * @file SvLog.h SvLog logging facility
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "SvAtomic.h"
#include "SvTime.h"
#include "SvLogColors.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvLog SvLog logging facility
 * @ingroup SvCore
 * @{
 *
 * Universal logging facility used by entire media platform.
 **/

#ifndef SvLogModuleCustomized
# undef SvLogModule
/**
 * Default definition of SvLogModule &mdash; the current file name.
 *
 * This value can be defined in source code
 * before including SvLog.h file in the following way:
 *
@code
  #define SvLogModule "com.yourcompany.module-name"
  #define SvLogModuleCustomized
@endcode
 **/
# define SvLogModule __FILE__
#endif


#ifndef SV_LOG_LEVEL
/**
 * Default logging level (minimal priority that can be logged).
 **/
# define SV_LOG_LEVEL 7
#endif


/**
 * Log priority levels.
 **/
typedef enum SvLogPriority_ {
    /// emergency level
    SvLogPriority_emergency = 1,
    /// alert level
    SvLogPriority_alert = 2,
    /// critical error level
    SvLogPriority_critical = 3,
    /// error level
    SvLogPriority_error = 4,
    /// warning level
    SvLogPriority_warning = 5,
    /// notice level
    SvLogPriority_notice = 6,
    /// informative level
    SvLogPriority_info = 7,
    /// debug level
    SvLogPriority_debug = 8,
} SvLogPriority;

/**
 * Logging plugin.
 **/
typedef struct SvLogPlugin_ {
    /// plugin name
    const char *name;
    /// logging function
    void (*writeMessage)(const char *prefix,
                         const char *msg);
} SvLogPlugin;

/**
 * Log context.
 *
 * Log context is used as a metadata source by logging functions:
 *
 *  - to decide if a message should be logged at all,
 *  - to provide additional information such as location in the source
 *    file where the message originates.
 **/
typedef struct SvLogContext_ {
    /**
     * cookie used to assist in filtering, defaults to function name
     **/
    const char *cookie;
    /**
     * module name used in filtering
     **/
    const char *module;
    /**
     * name of the source file
     **/
    const char *file;
    /**
     * line number in the source file
     **/
    int line;
    /**
     * priority level of the displayed messages
     **/
    SvLogPriority priority;
} SvLogContext;


/**
 * Get global logging level.
 *
 * @return global logging level configured at runtime
 **/
extern int
SvLogGetLevel(void);

/**
 * Register logging plugin.
 *
 * @param[in] plugin    plugin to be registered
 **/
extern void
SvLogRegisterPlugin(const SvLogPlugin *const plugin);

/**
 * Reset start time value used for displaying relative timestamps.
 **/
extern void
SvLogResetReferenceTime(void);

/**
 * Check if log message should be displayed according to current
 * runtime configuration.
 *
 * @param[in] context log context
 * @return @c true if message described by the @a context should
 *         be displayed, @c false if not
 *
 * @internal
 **/
extern bool
SvLogContextShouldDisplayMessage(const SvLogContext *context);

/**
 * Adjust nesting depth based on provided delta and context.
 *
 * @param[in] context log context
 * @param[in] delta the delta (usually @c +1 or @c -1)
 *
 * @internal
 **/
extern void
SvLogContextAdjustNestingLevelForCallingThread(SvLogContext *context,
                                               int delta);

/**
 * Log a message.
 *
 * @param[in] context log context
 * @param[in] message a printf-like format string
 * @param[in] ap argument list
 **/
extern void
SvLogMessageV(SvLogContext *context,
              const char *message,
              va_list ap);

/**
 * Log a message.
 *
 * @param[in] context log context
 * @param[in] message a printf-like format string
 **/
extern void __attribute__((format (printf, 2, 3)))
SvLogMessage(SvLogContext *context,
             const char *message, ...);

/**
 * Log a message without extra formatting (like adding
 * timestamps and line breaks.)
 *
 * @param[in] prio message priority
 * @param[in] message a printf-like format string
 **/
#if SV_LOG_LEVEL > 0
extern void
/** @cond */ __attribute__((format (printf, 2, 3))) /** @endcond */
SvLogRaw(SvLogPriority prio,
         const char *message, ...);
#else
static inline void
/** @cond */ __attribute__((format (printf, 2, 3))) /** @endcond */
SvLogRaw(SvLogPriority prio,
         const char *message, ...)
{
}
#endif

/**
 * Reset timestamps printed in "relative" mode back to zero.
 **/
extern void
SvLogResetRelativeTimestamp(void);

/**
 * @cond
 **/

/**
 * Defines a SvLogContext based on the current code location.
 *
 * @internal
 **/
#define SV_LOG_CONTEXT(contextName, priorityLevel) \
        SvLogContext contextName; \
        contextName.cookie   = __FUNCTION__; \
        contextName.module   = SvLogModule; \
        contextName.file     = __FILE__; \
        contextName.line     = __LINE__; \
        contextName.priority = (priorityLevel); \


static inline void __attribute__((unused))
SvLogWithFiltering_noop(SvLogPriority prio, ...)
{
}

/**
 * @endcond
 **/


#if SV_LOG_LEVEL > 0

/**
 * @cond
 **/
extern void __attribute__((format (printf, 4, 5)))
SvLogWithFiltering_(SvLogPriority prio,
                    const char *file, int line,
                    const char *fmt, ...);
/**
 * @endcond
 **/

/**
 * Log a message with given priority.
 *
 * @hideinitializer
 *
 * @param[in] prio message priority
 * @param[in] fmt a printf-like format string
 **/
# define SvLogWithFiltering(prio, fmt, ...) SvLogWithFiltering_((prio), __FILE__, __LINE__, (fmt), ## __VA_ARGS__)

#else

# define SvLogWithFiltering(prio, fmt, ...) SvLogWithFiltering_noop((prio), ## __VA_ARGS__)

#endif


/**
 * Macro useful for creating an "entering function" logging context.
 *
 * @note This macro must be paired with SvLeave()
 *
 * @hideinitializer
 **/
#if SV_LOG_LEVEL > 7
# define SvEnter(arg_format, ...) \
    do { \
        SV_LOG_CONTEXT(_context, SvLogPriority_debug); \
        if (SvLogContextShouldDisplayMessage(&_context)) \
            SvLogMessage(&_context, ">>> %s(" arg_format ")", __FUNCTION__, ## __VA_ARGS__ ); \
        SvLogContextAdjustNestingLevelForCallingThread(&_context, 1); \
    } while (0)

#else
# define SvEnter(...) do { } while (0)
#endif

/**
 * Macro useful for creating a "leaving function" logging context.
 *
 * @note This macro must be paired with SvEnter().
 *
 * @hideinitializer
 **/
#if SV_LOG_LEVEL > 7
# define SvLeave(arg_format, ...) \
    do { \
        SV_LOG_CONTEXT(_context, SvLogPriority_debug); \
        SvLogContextAdjustNestingLevelForCallingThread(&_context, -1); \
        if (SvLogContextShouldDisplayMessage(&_context)) \
            SvLogMessage(&_context, "<<< %s(), returning " arg_format, __FUNCTION__, ## __VA_ARGS__ ); \
    } while (0)
#else
# define SvLeave(...) do { } while (0)
#endif

/**
 * Log an emergency message.
 * @hideinitializer
 * @param[in] format a printf-like format string
 **/
#define SvLogEmergency(format, ...)     SvLogWithFiltering(SvLogPriority_emergency, format, ## __VA_ARGS__)

/**
 * Log an alert message.
 * @hideinitializer
 * @param[in] format a printf-like format string
 **/
#define SvLogAlert(format, ...)         SvLogWithFiltering(SvLogPriority_alert, format, ## __VA_ARGS__)

/**
 * Log a critical message.
 * @hideinitializer
 * @param[in] format a printf-like format string
 **/
#define SvLogCritical(format, ...)      SvLogWithFiltering(SvLogPriority_critical, format, ## __VA_ARGS__)

/**
 * Log an error message.
 * @hideinitializer
 * @param[in] format a printf-like format string
 **/
#define SvLogError(format, ...)         SvLogWithFiltering(SvLogPriority_error, format, ## __VA_ARGS__)

/**
 * Log a warning message.
 * @hideinitializer
 * @param[in] format a printf-like format string
 **/
#define SvLogWarning(format, ...)       SvLogWithFiltering(SvLogPriority_warning, format, ## __VA_ARGS__)

/**
 * Log a notice message.
 * @hideinitializer
 * @param[in] format a printf-like format string
 **/
#define SvLogNotice(format, ...)        SvLogWithFiltering(SvLogPriority_notice, format, ## __VA_ARGS__)

/**
 * Log an information message.
 * @hideinitializer
 * @param[in] format a printf-like format string
 **/
#define SvLogInfo(format, ...)          SvLogWithFiltering(SvLogPriority_info, format, ## __VA_ARGS__)

/**
 * Log a debugging message.
 * @hideinitializer
 * @param[in] format a printf-like format string
 **/
#if SV_LOG_LEVEL <= 7
# define SvLogDebug(format, ...)         SvLogWithFiltering_noop(SvLogPriority_debug, format, ## __VA_ARGS__)
#else
# define SvLogDebug(format, ...)         SvLogWithFiltering(SvLogPriority_debug, format, ## __VA_ARGS__)
#endif

/**
 * Log a trace message.
 **/
#define SvLogTrace()                    SvLogDebug("<TRACE> %s:%d (in %s())", __FILE__, __LINE__, __FUNCTION__)


/**
 * @}
 **/


// do not generate documentation for the rest of this file
/**
 * @cond
 **/

//-----------------------------------

#define DECL_DUMP_OUTPUT(NAME, ENV_NAME, DESC) \
static char* NAME##_dump_prefix = NULL; \
static int   NAME##_dump_fd = -1; \
static int   NAME##_dump_counter = -1; \
static char  NAME##_dump_file_name[256]; \
 \
static void NAME##_dump_env_helper_() __attribute__ ((constructor)); \
static void NAME##_dump_env_helper_() { \
  if (getenv("HELP") ) \
    SvLogNotice("Env string variable [%s] : %s", ENV_NAME, DESC); \
}; \
 \
static void NAME##_open_dump_file(void) \
{ \
  if (!NAME##_dump_prefix) { \
    NAME##_dump_prefix = getenv(ENV_NAME); \
    if (!NAME##_dump_prefix) \
      return; \
    NAME##_dump_prefix = strdup(NAME##_dump_prefix); \
}; \
 \
  NAME##_dump_counter++; \
  snprintf(NAME##_dump_file_name, sizeof(NAME##_dump_file_name), "%s_%d", NAME##_dump_prefix, NAME##_dump_counter); \
 \
  NAME##_dump_fd = open(NAME##_dump_file_name, O_RDWR|O_TRUNC|O_CREAT, 0666); \
  if (NAME##_dump_fd < 0) { \
    SvLogNotice(COLBEG() "Failure to open dump file [%s].\r\n" COLEND_COL(red), NAME##_dump_file_name); \
    return; \
}; \
  SvLogNotice(COLBEG() "Opened dump file [%s].\r\n" COLEND_COL(yellow), NAME##_dump_file_name); \
}; \
 \
static void NAME##_close_dump_file(void) \
{ \
  if (NAME##_dump_fd < 0) \
    return; \
  close(NAME##_dump_fd); \
  NAME##_dump_fd = -1; \
  SvLogNotice(COLBEG() "Closed dump file [%s].\r\n" COLEND_COL(yellow), NAME##_dump_file_name); \
}; \
 \
static void NAME##_dump_buf(void* buf, int len) \
{ \
  if (NAME##_dump_fd < 0) \
    return; \
  write(NAME##_dump_fd, buf, len); \
}; \

// Usage:
// DECL_DUMP_OUTPUT(udp, "UDP_DUMP_PREFIX", "dump packets received from udp socket (file prefix)");
// udp_close_dump_file();
// udp_open_dump_file();
// udp_dump_buf(buf, len);
//
// export UDP_DUMP_PREFIX="./udp_dumps/dump_"

/**
 * @endcond
 **/


#ifdef __cplusplus
}
#endif

#endif

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

#ifndef SV_ERROR_INFO_H_
#define SV_ERROR_INFO_H_

/**
 * @file SvErrorInfo.h SvErrorInfo class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <stdarg.h>
#include <SvCore/SvLog.h>


#ifndef likely
# define likely(x) __builtin_expect(!!(x), 1)
#endif


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvCoreErrorHandling Error handling
 * @ingroup SvCore
 * @{
 *
 * Functions that implement error handling based on SvErrorInfo type.
 **/

/**
 * The SvErrorInfo class.
 *
 * @note SvErrorInfo is \b not an SvObject!
 **/
typedef struct SvErrorInfo_ *SvErrorInfo;

/**
 * Create an error info object linked to other error info object as a root cause.
 *
 * This function creates an error info object with given error code, error domain
 * and root cause.
 *
 * The domain and code are simply copied to the error info object (the domain must
 * be a static C-string). The format string is used to create the message
 * property of the error info object.
 *
 * The cause error is linked as additional message parameter.
 *
 * The error info object must be discarded using SvErrorInfoDestroy().
 *
 * The error domain does not have to be registered prior to calling this
 * function. Registering an error domain is only necessary to translate error
 * code to error name useful for debugging.
 *
 * @param[in] domain error domain
 * @param[in] code error code specific to @a domain
 * @param[in] cause an (optional) error info instance that is the cause of this error
 * @param[in] format printf-like format string
 * @param[in] args argument list
 * @return new error info instance linked to the @a cause error
 **/
extern SvErrorInfo
SvErrorInfoCreateVWithCause(const char *domain,
                            int code,
                            SvErrorInfo cause,
                            const char *format,
                            va_list args);

/**
 * Create an error info object linked to other error info object as a root cause.
 *
 * This function is a variant of SvErrorInfoCreateVWithCause().
 * Created error info object has the same error domain and error code
 * as the @a cause, which is a mandatory argument.
 *
 * The error info object must be discarded using SvErrorInfoDestroy().
 *
 * @since 1.12.8
 *
 * @param[in] cause an error info instance that is the cause of this error
 * @param[in] format printf-like format string
 * @param[in] args argument list
 * @return new error info instance linked to the @a cause error
 **/
extern SvErrorInfo
SvErrorInfoCreateVWithExtendedInfo(SvErrorInfo cause,
                                   const char *format,
                                   va_list args);

/** @cond */

extern SvErrorInfo
SvErrorInfoCreateSimple(const char *domain,
                        int code,
                        SvErrorInfo cause);

/** @endcond */

/**
 * Tag error info object with source code location.
 *
 * @param[in] self error info
 * @param[in] function function name
 * @param[in] file source file name
 * @param[in] line line number in @a file
 **/
extern void
SvErrorInfoSetLocation(SvErrorInfo self,
                       const char *function,
                       const char *file,
                       int line);

#if SV_LOG_LEVEL > 0

/**
 * Create an error info object.
 *
 * This function is a convenience wrapper for SvErrorInfoCreateVWithCause().
 *
 * @param[in] domain error domain
 * @param[in] code error code specific to @a domain
 * @param[in] function function name
 * @param[in] file source file name
 * @param[in] line line number in source file
 * @param[in] format printf-like format string
 * @return new error info instance
 **/
extern SvErrorInfo
/** @cond */ __attribute__((format (printf, 6, 7))) /** @endcond */
SvErrorInfoCreate_(const char *domain,
                   int code,
                   const char *function,
                   const char *file,
                   int line,
                   const char *format,
                   ...);

/**
 * Create an error info object.
 *
 * This function is a convenience wrapper for SvErrorInfoCreateVWithCause().
 *
 * @param[in] domain error domain
 * @param[in] code error code specific to @a domain
 * @param[in] cause an (optional) error info instance that is the cause of this error
 * @param[in] function function name
 * @param[in] file source file name
 * @param[in] line line number in source file
 * @param[in] format printf-like format string
 * @return new error info instance linked to the @a cause error
 **/
extern SvErrorInfo
/** @cond */ __attribute__((format (printf, 7, 8))) /** @endcond */
SvErrorInfoCreateWithCause_(const char *domain,
                            int code,
                            SvErrorInfo cause,
                            const char *function,
                            const char *file,
                            int line,
                            const char *format,
                            ...);

/**
 * Create an error info object with the same error domain and code
 * as the original cause.
 *
 * @since 1.12.8
 *
 * @param[in] cause error info instance that is the cause of this error
 * @param[in] function function name
 * @param[in] file source file name
 * @param[in] line line number in source file
 * @param[in] format printf-like format string
 * @return new error info instance linked to the @a cause error
 **/
extern SvErrorInfo
/** @cond */ __attribute__((format (printf, 5, 6))) /** @endcond */
SvErrorInfoCreateWithExtendedInfo_(SvErrorInfo cause,
                                   const char *function,
                                   const char *file,
                                   int line,
                                   const char *format,
                                   ...);

# define SvErrorInfoCreate(domain, code, fmt, ...) \
    SvErrorInfoCreate_((domain), (code), __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
# define SvErrorInfoCreateWithCause(domain, code, cause, fmt, ...) \
    SvErrorInfoCreateWithCause_((domain), (code), (cause), __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
# define SvErrorInfoCreateWithExtendedInfo(cause, fmt, ...) \
    SvErrorInfoCreateWithExtendedInfo_((cause), __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#else // SV_LOG_LEVEL > 0

static inline SvErrorInfo __attribute__ ((unused))
SvErrorInfoCreate_noop(SvErrorInfo cause, const char* fmt, ...)
{
    return cause;
}

# define SvErrorInfoCreate(domain, code, fmt, ...) \
    SvErrorInfoCreateSimple((domain), (code), SvErrorInfoCreate_noop(NULL, fmt, ##__VA_ARGS__))
# define SvErrorInfoCreateWithCause(domain, code, cause, fmt, ...) \
    SvErrorInfoCreateSimple((domain), (code), SvErrorInfoCreate_noop((cause), fmt, ##__VA_ARGS__))
# define SvErrorInfoCreateWithExtendedInfo(cause, fmt, ...) \
    SvErrorInfoCreate_noop((cause), fmt, ##__VA_ARGS__)

#endif

/**
 * Destroy an error info along with any cause errors.
 *
 * @param[in] self error info
 **/
extern void
SvErrorInfoDestroy(SvErrorInfo self);

/** @cond */

extern void
SvErrorInfoPropagate_(SvErrorInfo self,
                      SvErrorInfo *holder);

/** @endcond */

/**
 * Propagate the error info to the caller.
 *
 * @param[in] self error info
 * @param[in] holder a pointer to the variable that will contain the
 *                  propagated error, can be @c NULL
 **/
static inline void
SvErrorInfoPropagate(SvErrorInfo self,
                     SvErrorInfo* holder)
{
    if (likely(!self)) {
        if (holder)
            *holder = NULL;
    } else {
        SvErrorInfoPropagate_(self, holder);
    }
}

/**
 * Log the occurance of an error with message information.
 *
 * @param[in] self error info
 **/
extern void
SvErrorInfoWriteLogMessage(SvErrorInfo self);

/**
 * Register an error domain using translation table.
 *
 * @param[in] name name of the registered error domain (should follow reverse domain name scheme)
 * @param[in] lastError the largest valid error number + 1 (size of @a errorNames array)
 * @param[in] errorNames an array of error names for all the error codes in this domain
 **/
extern void
SvErrorInfoRegisterDomain(const char *name,
                          int lastError,
                          const char **errorNames);

/**
 * Register an error domain using translation function.
 *
 * @param[in] name name of the registered error domain (should follow reverse domain name scheme)
 * @param[in] strerrorFn a function for translating error codes to error names
 **/
extern void
SvErrorInfoRegisterDomainWithFun(const char *name,
                                 const char *(*strerrorFn)(int));

/**
 * Get the error code from an error info instance.
 *
 * @param[in] self error info
 * @return error code
 **/
extern int
SvErrorInfoGetCode(SvErrorInfo self);

/**
 * Get the error domain from an error info instance.
 *
 * @note Do not free the return value. Its lifetime is valid forever.
 *
 * @param[in] self error info
 * @return error domain
 **/
extern const char *
SvErrorInfoGetDomain(SvErrorInfo self);

/**
 * Get the error message from an error info instance.
 *
 * @note Do not free the return value. Its lifetime is bound to the lifetime of
 * the error info instance.
 *
 * @param[in] self error info
 * @return error message
 **/
extern const char *
SvErrorInfoGetMessage(SvErrorInfo self);

/**
 * Get the cause error from an error info instance.
 *
 * This method can be used to inspect the root cause of some errors.
 *
 * @note This function returns @c NULL if the error has no embedded error cause
 * information. The embedded error cannot be destroyed separately, so it's an
 * error to pass that to SvErrorInfoDestroy().
 *
 * @note Do not free the return value! Its lifetime is bound to the lifetime of
 * the external error info instance.
 *
 * @param[in] self error info
 * @return cause error info, @c NULL if not present
 **/
extern SvErrorInfo
SvErrorInfoGetCause(SvErrorInfo self);

/**
 * Check if error info instance is from given error domain.
 *
 * @since 1.12.7
 *
 * @param[in] self error info
 * @param[in] domain error domain
 * @return @c true if @a self is from @a domain, @c false otherwise
 **/
extern bool
SvErrorInfoIsFromDomain(SvErrorInfo self,
                        const char *domain);

/**
 * Check if error info matches given error code and domain.
 *
 * @since 1.12.7
 *
 * @param[in] self error info
 * @param[in] domain error domain
 * @param[in] code error code specific to @a domain
 * @return @c true if @a domain and @a code match @a self, otherwise @c false
 **/
extern bool
SvErrorInfoEquals(SvErrorInfo self,
                  const char *domain,
                  unsigned int code);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

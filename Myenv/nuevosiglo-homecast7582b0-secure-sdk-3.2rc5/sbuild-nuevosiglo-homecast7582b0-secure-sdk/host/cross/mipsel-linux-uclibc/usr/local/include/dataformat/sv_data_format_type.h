/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
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

#ifndef SV_DATA_FORMAT_TYPE_H
#define SV_DATA_FORMAT_TYPE_H

struct svdataformat;

#ifdef __cplusplus
extern "C" {
#endif

void svdataformat_destroy(struct svdataformat* format);
struct svdataformat* svdataformat_copy_(const struct svdataformat* format, const char* file, const char* function, int line);

struct svdataformat* svdataformat_retain_(const struct svdataformat* format_const, const char* file, const char* function, int line);
void svdataformat_release_(const struct svdataformat* format, const char* file, const char* function, int line);


#if SV_LOG_LEVEL > 0

#define svdataformat_retain(_format_const_)  svdataformat_retain_( _format_const_, __FILE__, __FUNCTION__, __LINE__)
#define svdataformat_release(_format_const_) svdataformat_release_(_format_const_, __FILE__, __FUNCTION__, __LINE__)
#define svdataformat_copy(other)             svdataformat_copy_(other, __FILE__, __FUNCTION__, __LINE__)

void svdataformat_printf(const struct svdataformat* format, const char* reason);

#else

#define svdataformat_retain(_format_const_)  svdataformat_retain_( _format_const_, "", "", 0)
#define svdataformat_release(_format_const_) svdataformat_release_(_format_const_, "", "", 0)
#define svdataformat_copy(other)             svdataformat_copy_(other, "", "", 0)

static inline void __attribute__((unused))
svdataformat_printf(const struct svdataformat* format, const char* reason)
{
}

#endif


#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_DATA_FORMAT_TYPE_H

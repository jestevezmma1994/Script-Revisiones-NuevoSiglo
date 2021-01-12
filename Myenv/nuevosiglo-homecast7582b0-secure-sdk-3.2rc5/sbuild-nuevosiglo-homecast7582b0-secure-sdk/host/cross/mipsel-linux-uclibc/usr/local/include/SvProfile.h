/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SV_PROFILE_H
#define SV_PROFILE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: docs!

typedef struct SvTimerLocation* SvTimerLocation_t;
struct SvTimerLocation;

struct SvTimerLocation* SvTimerLocationCreate(const char* name, int id);
void SvTimerLocationDestroy(SvTimerLocation_t loc);

void SvProfileStart(SvTimerLocation_t loc);
void SvProfileStop(SvTimerLocation_t loc);
void SvProfileBlock(void);
void SvProfileUnblock(void);

void SvProfileIgnoreSubNodes(bool ignore);

void SvProfileInit(void);
void SvProfilePrint(struct SvTimerLocation* loc);
void SvProfileCleanup(void);


#if (defined SV_PROFILE_DISABLED)

#define PROFILE_FUN_ENTER() do {} while (0)
#define PROFILE_FUN_LEAVE() do {} while (0)
#define PROFILE_ENTER(NAME) do {} while (0)
#define PROFILE_LEAVE() do {} while (0)

#else

#define PROFILE_FUN_ENTER() \
  do { \
    static struct SvTimerLocation* _fun_loc_ = NULL; \
    do { \
      if (!_fun_loc_) _fun_loc_ = SvTimerLocationCreate(__FUNCTION__, __LINE__); \
      SvProfileStart(_fun_loc_); \
    } while (0)

#define PROFILE_FUN_LEAVE() \
    do { \
      SvProfileStop(_fun_loc_); \
    } while (0); \
  } while (0)

#define PROFILE_ENTER(NAME) \
  do { \
    static struct SvTimerLocation* _fun_loc_ = NULL; \
    do { \
      if (!_fun_loc_) _fun_loc_ = SvTimerLocationCreate(NAME, __LINE__); \
      SvProfileStart(_fun_loc_); \
    } while (0)

#define PROFILE_LEAVE() \
    do { \
      SvProfileStop(_fun_loc_); \
    } while (0); \
  } while (0)


#define PROFILE_ENTER_EACH(NAME) \
  do { \
    static int _enter_cnt_ = 0; \
    struct SvTimerLocation* _fun_loc_ = NULL; \
    do { \
      if (!_fun_loc_) _fun_loc_ = SvTimerLocationCreate(NAME, _enter_cnt_++); \
      SvProfileStart(_fun_loc_); \
    } while (0)

#endif // (defined SV_PROFILE_DISABLED)


#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_PROFILE_H

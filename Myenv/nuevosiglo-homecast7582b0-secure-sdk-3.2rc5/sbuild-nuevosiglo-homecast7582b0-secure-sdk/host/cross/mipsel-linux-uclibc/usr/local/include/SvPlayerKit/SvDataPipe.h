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

#ifndef SV_DATA_PIPE_H_
#define SV_DATA_PIPE_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvPlayerKit/SvChbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SvScheduler_s;

struct SvDataPipe_s;
typedef struct SvDataPipe_s* SvDataPipe;

struct SvDataPipeCallbacks
{
  void (*data) (void* owner, SvDataPipe dp, SvChbuf chdata);
};

struct SvDataPipe_s
{
  void (*push)   (SvDataPipe dp, SvChbuf chdata);
  void (*flush)  (SvDataPipe dp);

  const struct SvDataPipeCallbacks* callbacks;
  void* owner;
};


static inline void SvDataPipeSetCallbacks(SvDataPipe dp, const struct SvDataPipeCallbacks* callbacks, void* owner) {
  dp->callbacks = callbacks;
  dp->owner = owner;
};
static inline void SvDataPipePush(SvDataPipe dp, SvChbuf chdata) {
  dp->push(dp, chdata);
};
static inline void SvDataPipeFlush(SvDataPipe dp) {
  dp->flush(dp);
};


// ---------------------------------------

/// Function should process data from \a in, putting used buffers into \a trash,
/// and the results in \a out.
/// Function should break the processing when time slice of its fiber has passed.
typedef void (SvDataPipeTransformFun)(SvObject arg, SvChbuf in, SvChbuf trash, SvChbuf out);

/**
 * \param schedExternal Callbacks will be issued from this scheduler's context.
 *                      Also, pushed buffers will be released from context of this scheduler.
 * \param schedInternal Processing will be done in this scheduler's context.
 */
SvDataPipe  SvDataPipeThreadedCreate_(SvString name,
                                      struct SvScheduler_s* schedExternal,
                                      struct SvScheduler_s* schedInternal,
                                      SvDataPipeTransformFun* fun,
                                      SvObject arg);
void SvDataPipeThreadedDestroy(SvDataPipe dp);

SvDataPipe  SvDataPipeThreadedCreate(SvString name,
                                     struct SvScheduler_s* schedExternal,
                                     SvDataPipeTransformFun* fun,
                                     SvObject arg);

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_DATA_PIPE_H

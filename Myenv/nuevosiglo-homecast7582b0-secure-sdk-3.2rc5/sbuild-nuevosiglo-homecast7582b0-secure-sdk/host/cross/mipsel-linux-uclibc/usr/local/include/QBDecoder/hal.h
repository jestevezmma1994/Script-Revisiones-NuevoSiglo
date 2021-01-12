/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_DECODER_HAL_H
#define QB_DECODER_HAL_H

#include <QBDecoder.h>

#include <QBViewport.h>
#include <QBDemux.h>
#include <dataformat/sv_data_format.h>
#include <SvPlayerKit/SvChbuf.h>
#include <SvPlayerKit/SvClosedCaptionSubtitles.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int QBDecoderHALCreate(QBDecoder** out,
                       const QBDecoderCreateSettings* createSettings);

int QBDecoderHALSetupAsync(QBDecoder* dec);

void QBDecoderHALClose(QBDecoder* dec);
int  QBDecoderHALCloseAsync(QBDecoder* dec);

int QBDecoderHALStopAsync(QBDecoder* dec);
int QBDecoderHALPlayAsync(QBDecoder* dec, int nom, int denom);

int QBDecoderHALUpdateAsync(QBDecoder* dec);
void QBDecoderHALIdleAsync(QBDecoder* dec);

int QBDecoderHALAnalyzeFormat(const QBDecoder* dec, struct svdataformat* format, int* videoCntOut, int* audioCntOut);

/// ts or asf
int QBDecoderHALPushStream(QBDecoder* dec, SvChbuf chdata);

/// only if pushing pure es (quirks.pes.noPesheader)
int QBDecoderHALStartFrame(QBDecoder* dec, bool isVideo, uint64_t pts, unsigned int total_len);
/// fragments of pes, or es
int QBDecoderHALPushFrameFragment(QBDecoder* dec, bool isVideo, struct SvChbufCtx* ctx, unsigned int len, bool oob);
/// notify that all fragments from PushFrameFragment() have been pushed
int QBDecoderHALFinishFrame(QBDecoder* dec, bool isVideo);

int QBDecoderHALCanPush(QBDecoder* dec, int purpose);
int QBDecoderHALCanPushWithDataLength(QBDecoder* dec, int purpose, const unsigned int dataLength);
int QBDecoderHALIsEos(QBDecoder* dec);

int QBDecoderHALGetClockForDirectAudio(QBDecoder* dec, int64_t* durationPlayed, int64_t* stc90k);

QBDemux* QBDecoderHALGetDemux(const QBDecoder* dec);

int QBDecoderHALReadClosedCaption(QBDecoder* dec, QBClosedCaptionData *data, unsigned int max_len);

void QBDecoderHALPrintStats(QBDecoder* dec);

/**
 * Destroy container that keeps lifetime variables required by some HALs.
 */
void QBDecoderHALDestroyContainer(void);

typedef struct QBDecoderQuirks_s  QBDecoderQuirks;
struct QBDecoderQuirks_s
{
    struct {
        bool noPesHeader;
    } pes;
    struct {
        bool  needAdtsHeaders;
    } aac;
    struct {
        bool  addToEveryPes;
    } videoHeader;
};
const QBDecoderQuirks* QBDecoderHALGetQuirks(QBDecoder* dec);


#ifdef __cplusplus
}
#endif


#endif // #ifndef QB_DECODER_HAL_H

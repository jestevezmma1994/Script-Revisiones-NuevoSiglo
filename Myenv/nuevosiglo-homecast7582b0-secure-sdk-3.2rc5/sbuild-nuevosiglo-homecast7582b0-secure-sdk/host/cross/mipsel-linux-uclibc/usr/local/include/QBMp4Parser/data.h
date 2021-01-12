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

#ifndef QB_MP4_PARSER_DATA_H
#define QB_MP4_PARSER_DATA_H

#include <stdint.h>
#include <stdbool.h>

#include <dataformat/audio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum QBMp4TrackType_e  QBMp4TrackType;
enum QBMp4TrackType_e
{
  QBMp4TrackType_unknown = 0,

  QBMp4TrackType_video   = 1,
  QBMp4TrackType_audio   = 2,

  QBMp4TrackType_other = 0xff,
};

const char* QBMp4TrackTypeToString(QBMp4TrackType type);


struct DecodingTimeToSample_s
{
  uint32_t cnt;   /// <cnt> samples follow - each one with ...
  uint32_t delta; /// ... duration <delta>
};
struct CompositionTimeToSample_s
{
  uint32_t cnt;    /// <cnt> samples follow - each one with ...
  int32_t offset; /// ... pts - dts = <offset>
};
struct SampleToChunk_s
{
  uint32_t first_chunk;               /// first chunk of set of consecutive chunks - each one with ...
  uint32_t samples_per_chunk;         /// ... <samples_per_chunk> samples
  uint32_t sample_description_index;  /// ???
};


typedef struct QBMp4Track_s  QBMp4Track;
struct QBMp4Track_s
{
  uint32_t  id;
  QBMp4TrackType  type;

  bool  is_enabled;
  bool  in_movie;
  bool  in_preview;

  uint64_t  duration;
  uint32_t  timescale;
  char lang[4];

  uint8_t* private_data;
  uint32_t private_data_len;

  struct {
    // TrackHeader
    uint32_t w;
    uint32_t h;
    uint16_t depth;
    uint16_t layer;
    // specific
    uint32_t fourcc;
  } video;

  struct {
    // TrackHeader
    uint16_t volume;
    uint16_t balance;
    uint16_t channels;
    uint16_t sample_size;
    uint32_t sample_rate;
    // specific
    QBAudioCodec codec;
  } audio;

  struct {
    uint32_t buffer_size;
    uint32_t max_bitrate;
    uint32_t avg_bitrate;
  } buffers;

  //--------------------
  // tables
  //--------------------

  uint32_t  DecodingTimeToSampleCnt; /// many (almost one per sample), or just one
  struct DecodingTimeToSample_s* DecodingTimeToSample;

  uint32_t  CompositionTimeToSampleCnt; /// many (almost one per sample), or none at all
  struct CompositionTimeToSample_s* CompositionTimeToSample;

  uint32_t  SyncSampleCnt; /// only for video tracks (for audio, every sample is a sync sample)
  uint32_t* SyncSample;

  uint32_t  ChunkOffsetCnt; /// ~ sample cnt / X
  uint32_t* ChunkOffset;
  uint64_t* ChunkLargeOffset;

  uint32_t  SampleToChunkCnt; /// any number from 1 to chunk cnt
  struct SampleToChunk_s* SampleToChunk;

  uint32_t  SampleSizeCnt; /// sample cnt, or zero iff SampleSizeCommon > 0
  uint32_t* SampleSize;
  uint32_t  SampleSizeCommon;
};

QBMp4Track* QBMp4TrackAlloc(void);
void QBMp4TrackDestroy(QBMp4Track* track);


uint32_t QBMp4TrackGetSampleFromDts(const QBMp4Track* track, uint64_t dt90khz);
uint64_t QBMp4TrackGetDtsFromSample(const QBMp4Track* track, uint32_t sample_idx, uint32_t* delta_out);

uint32_t QBMp4TrackGetNearestSyncSample(const QBMp4Track* track, uint32_t sample_idx);
uint64_t QBMp4TrackGetSampleOffset(const QBMp4Track* track, uint32_t chunk_idx, uint32_t sample_idx, uint32_t chunk_sample_idx);

uint32_t QBMp4TrackFindNextChunkFromOffset(const QBMp4Track* track, uint64_t* offset);

uint64_t QBMp4TrackGetChunkOffset(const QBMp4Track* track, uint32_t chunk_idx);


typedef struct QBMp4Info_s  QBMp4Info;
struct QBMp4Info_s
{
  uint64_t  duration;
  uint32_t  timescale;
  uint16_t  volume;

  uint64_t  mdat_pos;
  uint64_t  mdat_len;

  uint32_t  track_cnt;
  uint32_t  total_track_cnt;
  QBMp4Track** tracks;
};

void QBMp4InfoDestroy(QBMp4Info* info);
void QBMp4InfoPrint(const QBMp4Info* info, const char* prefix);

QBMp4Info* QBMp4InfoAlloc(void);
void QBMp4InfoAddTrack(QBMp4Info* info, QBMp4Track* track);

void QBMp4InfoNormalize(QBMp4Info* info);

struct QBMp4SampleEncryptionEntry_s {
    uint16_t clear_bytes;
    uint32_t encrypted_bytes;
};

struct QBMp4SampleEncryptionInfo_s {
    uint8_t iv[8];
    uint16_t entry_count;
    struct QBMp4SampleEncryptionEntry_s *entries;
};


union QBMp4FragmentReferenceEntry {
    struct {
        uint64_t start, duration;
    } v1;
    struct {
        uint32_t start, duration;
    } v0;
};

struct QBMp4FragmentReferenceInfo_s {
    uint8_t version;
    uint8_t count;

    union QBMp4FragmentReferenceEntry entries[0];
};





typedef struct QBMp4FragmentInfo_s  QBMp4FragmentInfo;
struct QBMp4FragmentInfo_s
{
  uint32_t  seq_num;
  uint32_t  track_id;

  uint64_t  data_offset;
  uint32_t  first_sample_flags;

  uint32_t  default_sample_duration;
  uint32_t  default_sample_size;
  uint32_t  default_sample_flags;

  uint32_t  sample_cnt;

  uint32_t* duration_tab;
  uint32_t* size_tab;
  uint32_t* flags_tab;
  int32_t*  ctto_tab;

  uint64_t  mdat_pos;
  uint64_t  mdat_len;

  uint32_t  sample_enc_cnt;
  struct QBMp4SampleEncryptionInfo_s *sample_enc_tab;

  struct QBMp4FragmentReferenceInfo_s *fragment_reference;
};

QBMp4FragmentInfo* QBMp4FragmentInfoAlloc(void);
void QBMp4FragmentInfoDestroy(QBMp4FragmentInfo* frag);

void QBMp4FragmentInfoPrint(const QBMp4FragmentInfo* frag, const char* prefix);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_MP4_PARSER_DATA_H

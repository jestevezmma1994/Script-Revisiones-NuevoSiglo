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

#ifndef QB_MP4_PARSER_DATA_H
#define QB_MP4_PARSER_DATA_H

#include <stdint.h>
#include <stdbool.h>

#include <dataformat/audio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum QBMp4TrackType_e QBMp4TrackType;
enum QBMp4TrackType_e {
    QBMp4TrackType_unknown = 0,

    QBMp4TrackType_video   = 1,
    QBMp4TrackType_audio   = 2,

    QBMp4TrackType_other = 0xff,
};

extern const char *
QBMp4TrackTypeToString(QBMp4TrackType type);


struct DecodingTimeToSample_s {
    uint32_t cnt;   /// \<cnt\> samples follow - each one with ...
    uint32_t delta; /// ... duration \<delta\>
};
struct CompositionTimeToSample_s {
    uint32_t cnt;    /// \<cnt\> samples follow - each one with ...
    int32_t offset; /// ... pts - dts = \<offset\>
};
struct SampleToChunk_s {
    uint32_t first_chunk;               /// first chunk of set of consecutive chunks - each one with ...
    uint32_t samples_per_chunk;         /// ... \<samples_per_chunk\> samples
    uint32_t sample_description_index;  /// ???
};

typedef struct QBMp4Track_s QBMp4Track;
struct QBMp4Track_s {
    uint32_t id;
    QBMp4TrackType type;

    bool is_enabled;
    bool in_movie;
    bool in_preview;

    uint64_t duration;
    uint32_t timescale;
    char lang[4];

    uint8_t *private_data;
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

    uint32_t scheme_fourcc;      /// protection or encryption scheme 4CC code

    // track encryption
    uint32_t  is_encrypted;      /// encryption state of samples
    uint8_t   IV_size;          /// Initialization Vector size in bytes
    uint8_t   KID[16];          /// key identifier

    //--------------------
    // tables
    //--------------------

    uint32_t DecodingTimeToSampleCnt; /// many (almost one per sample), or just one
    struct DecodingTimeToSample_s *DecodingTimeToSample;

    uint32_t CompositionTimeToSampleCnt; /// many (almost one per sample), or none at all
    struct CompositionTimeToSample_s *CompositionTimeToSample;

    uint32_t SyncSampleCnt; /// only for video tracks (for audio, every sample is a sync sample)
    uint32_t *SyncSample;

    uint32_t ChunkOffsetCnt; /// ~ sample cnt / X
    uint32_t *ChunkOffset;
    uint64_t *ChunkLargeOffset;

    uint32_t SampleToChunkCnt; /// any number from 1 to chunk cnt
    struct SampleToChunk_s *SampleToChunk;

    uint32_t SampleSizeCnt; /// sample cnt, or zero iff SampleSizeCommon > 0
    uint32_t *SampleSize;
    uint32_t SampleSizeCommon;
};

extern QBMp4Track *QBMp4TrackAlloc(void);
extern void QBMp4TrackDestroy(QBMp4Track *track);


extern uint32_t QBMp4TrackGetSampleFromDts(const QBMp4Track *track, uint64_t dt90khz);
extern uint64_t QBMp4TrackGetDtsFromSample(const QBMp4Track *track, uint32_t sample_idx, uint32_t *delta_out);

extern uint32_t QBMp4TrackGetNearestSyncSample(const QBMp4Track *track, uint32_t sample_idx);
extern uint64_t QBMp4TrackGetSampleOffset(const QBMp4Track *track, uint32_t chunk_idx, uint32_t sample_idx, uint32_t chunk_sample_idx);

extern uint32_t QBMp4TrackFindNextChunkFromOffset(const QBMp4Track *track, uint64_t *offset);

extern uint64_t QBMp4TrackGetChunkOffset(const QBMp4Track *track, uint32_t chunk_idx);

struct QBMp4SampleEncryptionEntry_s {
    uint16_t clear_bytes;
    uint32_t encrypted_bytes;
};

struct QBMp4SampleEncryptionInfo_s {
    uint8_t iv[16];
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

typedef struct QBMp4DefaultSampleParams_ {
    uint32_t descriptionIndex;
    uint32_t duration;
    uint32_t size;
    uint32_t flags;
} QBMp4DefaultSampleParams;

/**
 * Mp4 Segment Index Reference structure
 **/
struct QBMp4SegmentIndexReference_ {
    /// indicates whether the reference points directly to the media bytes
    /// of a referenced leaf subsegment, or to a Segment Index box that describes
    /// how the referenced subsegment is further subdivided; as a result,
    /// the segment may be indexed in a ‘hierarchical’ or ‘daisy-chain’ or other
    /// form by documenting time and byte offset information for other Segment Index
    /// boxes applying to portions of the same (sub)segment.
    bool reference_type;
    /// size of referenced subsegment
    uint32_t referenced_size;
    /// duration of a subsegment
    uint32_t subsegment_duration;
    /// indicates of the subsegment starts with a Stream Access Point
    bool starts_with_SAP;
    /// type of the Stream Access Points
    uint8_t SAP_type;
    /// specifies delta time of the Stream Access Point
    uint32_t SAP_delta_time;
};

/**
 * Segment Index box provides information about a single media stream of the Segment,
 * referred to as the reference stream. If provided, the first Segment Index box in a segment,
 * for a given media stream, shall document the entirety of that media stream in the segment,
 * and shall precede any other Segment Index box in the segment for the same media stream.
 **/
struct QBMp4SegmentIndexInfo_ {
    uint32_t reference_id;
    /// timescale of the segment
    uint32_t timescale;
    // earliest presentation time of the segment
    uint64_t earliest_presentation_time;
    /// separates Segment Index boxes from the media that they refer to
    uint64_t first_offset;
    /// number of entries in references_tab
    uint16_t reference_cnt;
    /// a sequence of references to subsegments of the (sub)segment documented by the box
    struct QBMp4SegmentIndexReference_ *references_tab;
};

typedef struct QBMp4FragmentInfo_s QBMp4FragmentInfo;
struct QBMp4FragmentInfo_s {
    uint32_t seq_num;
    uint32_t track_id;

    uint64_t data_offset;
    uint32_t first_sample_flags;

    QBMp4DefaultSampleParams default_sample_params;

    uint32_t sample_cnt;

    uint32_t *duration_tab;
    uint32_t *size_tab;
    uint32_t *flags_tab;
    int32_t *ctto_tab;

    uint64_t mdat_pos;
    uint64_t mdat_len;

    uint32_t sample_enc_cnt;
    struct QBMp4SampleEncryptionInfo_s *sample_enc_tab;

    struct QBMp4FragmentReferenceInfo_s *fragment_reference;
    /// Segment Index Info
    struct QBMp4SegmentIndexInfo_ *segment_index;

    /// Track Fragment Decode Time
    uint64_t track_frag_decode_time;
};

extern void
QBMp4FragmentInfoPrint(const QBMp4FragmentInfo *frag, const char *prefix);

typedef struct QBMp4Info_s QBMp4Info;
struct QBMp4Info_s {
    uint64_t duration;
    uint32_t timescale;
    uint16_t volume;

    uint64_t mdat_pos;
    uint64_t mdat_len;

    uint32_t track_cnt;
    uint32_t total_track_cnt;
    QBMp4Track * *tracks;
    QBMp4FragmentInfo *frag;
};

extern void QBMp4InfoDestroy(QBMp4Info *info);
extern void QBMp4InfoPrint(const QBMp4Info *info, const char *prefix);

extern QBMp4Info *QBMp4InfoAlloc(bool fragmentMode);
extern void QBMp4InfoAddTrack(QBMp4Info *info, QBMp4Track *track);

/**
 * Get track with given ID.
 *
 * @param info MP4 information structure
 * @param track_id ID of track to get
 * @return MP4 track structure
 */
extern QBMp4Track *QBMp4InfoGetTrack(QBMp4Info *info, uint32_t track_id);

extern void QBMp4InfoNormalize(QBMp4Info *info);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_MP4_PARSER_DATA_H

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

#ifndef QB_DEMUX_FUNCTOR_H
#define QB_DEMUX_FUNCTOR_H

#include "QBDemuxTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


struct QBDemuxFunctor {
    /** Close demux instance.
     */
    void (*close)(QBDemux* demux);

    /** Push packets from s/w memory to h/w engine.
        Only supported for some demux instances.
        \returns 0 on success, negative on error.
     */
    int (*push_packets)(QBDemux* demux, const unsigned char* packets, unsigned int packetCnt);

    /** Monitor PCR on given pid, and act accordingly to maintain pcr/stc sync.
     *  \returns 0 on success, SV_ERR_* on error.
     */
    int (*handle_pcr)(QBDemux* demux, int pcr_pid);

    /** Create pid channel on demux.
     *
     *  @param[in] demux QB Demux handle
     *  @param[in] pid pid that is to be filtered by this channel
     *  @param[in] buffer_size size of internal buffer
     *  @param[in] exclusive create demux channel with exclusive resources (used for rescrambling)
     *  @param[in] name name of channel
     *  @param[out] channel_out created channel handle
     *  @return 0 on success, SV_ERR_* on error.
     */
    int (*channel_create)(QBDemux* demux, int pid, int buffer_size, bool exclusive, const char* name, QBDemuxChannel** channel_out);

    /** Check if channel with hardware capabilities to read multiple pids into one buffer can be created.
     *  \returns \a true if hardware multi pid channel can be created, \a false otherwise.
     */
    bool (*has_hw_multi_channels)(QBDemux* demux);

    /** Create pid channel on \a demux.
     *  This channel should use hardware capabilities, to read multiple pids into one buffer.
     *  This channel is initially connected to no pids.
     *  If this demux does not support this kind of behaviour - this function returns 0, and sets \a channel_out to NULL.
     *  \param total_buffer_size  size of internal buffer
     *  \param max_pids  maximal number of pids to be received by this hw channel
     *  \returns 0 on success, SV_ERR_* on error.
     */
    int (*hw_multi_channel_create)(QBDemux* demux, int total_buffer_size, int max_pids, QBDemuxChannel** channel_out);

    /** Add another pid to multi-channel.
     * \param[in] channel  must be created with \a hw_multi_channel_create
     * \param[in] pid pid that is to be filtered by this channel
     * \param[in] exclusive create demux channel with exclusive resources (used for rescrambling)
     * \returns 0 on success, SV_ERR_* on error.
     */
    int (*hw_multi_channel_add_pid)(QBDemuxChannel* channel, int pid, bool exclusive);

    /** Remove one of the pids added to multi-channel with \a hw_multi_channel_add_pid.
     * \param channel  must be created with \a hw_multi_channel_create
     * \returns 0 on success, SV_ERR_* on error.
     */
    int (*hw_multi_channel_remove_pid)(QBDemuxChannel* channel, int pid);
};


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DEMUX_FUNCTOR_H

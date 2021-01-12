/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2005 Sentivision K.K. All rights reserved.
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

#ifndef SV_TUNER_RECV_H
#define SV_TUNER_RECV_H

#include <stdbool.h>
#include <sys/uio.h> // needed for "struct iovec".
#include <dataformat/sv_data_format.h>

#include <QBTunerTypes.h>
#include <QBDemuxTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------
// for player
//------------------------

struct sv_tuner_recv;

struct sv_tuner_recv_params {
    int tuner_num;

    /// service id
    int sid;
    /// If re-tune is needed, unlink all other receivers.
    bool force_unlink;

    /**
     * Callback fired when new pid is added to received data
     *
     * @param[in] self_ callbackData
     * @param[in] recv tuner receiver handle
     * @param[in] pid added pid
     * @return @c 0 on success, value < 0 on failure
     */
    int (*onPidAdded)(void *self_, struct sv_tuner_recv *recv, int pid);

    /**
     * Callback fired when pid is removed from received data
     *
     * @param[in] self_ callbackData
     * @param[in] recv tuner receiver handle
     * @param[in] pid removed pid
     * @return @c 0 on success, value < 0 on failure
     */
    int (*onPidRemoved)(void *self_, struct sv_tuner_recv *recv, int pid);

    /**
     * Callback fired when new pmt is detected
     *
     * @param[in] self_ callbackData
     * @param[in] recv tuner receiver handle
     * @return @c 0 on success, value < 0 on failure
     */
    int (*onNewPmt)(void *self_, struct sv_tuner_recv *recv);
    /// First param to callback
    void* callbackData;
    /// do not include pmt in received data
    bool excludePmt;

    /// used to allocate not shared demux channel resources
    bool exclusivePIDsAllocation;
};

int sv_tuner_recv_create(const struct sv_tuner_recv_params* params,
                         const struct QBTunerParams* freq_params,
                         int pid_cnt, const int* pid_tab, const int* pid_size_tab,
                         struct sv_tuner_recv** recv_out);

void sv_tuner_recv_destroy(struct sv_tuner_recv* recv);

void sv_tuner_recv_handle_pat_changes(struct sv_tuner_recv* recv);
int sv_tuner_recv_enable_pcr(struct sv_tuner_recv* recv);

/**
 * @param iov_cnt can be 1 or 2.
 * @returns amount of bytes read.
 * @returns blocking SV_ERR_*
 * @returns SV_ERR_RESOURCE_LIMIT when the reader was unlinked from its tuner
 * @returns fatal errors.
 */
int sv_tuner_recv_read(struct sv_tuner_recv* recv, struct iovec* iov, int iov_cnt);

int sv_tuner_recv_flush(struct sv_tuner_recv* recv);

/**
 * Get demux channel associated to given pid
 *
 * @param[in] recv tuner receiver handle
 * @param[in] pid pid number of channel
 * @param[out] channel demux channel associated to pid
 * @return @c 0 on success, value < 0 on failure
 */
int sv_tuner_recv_get_channel(struct sv_tuner_recv* recv, int pid, QBDemuxChannel **channel);

/**
 * Get maximum possible number of received pids
 *
 * @param[in] recv tuner receiver handle
 * @return maximum possible number of received pids
 */
int sv_tuner_recv_get_max_pids(struct sv_tuner_recv* recv);

/**
 * Get information about which pids are received
 *
 * @param[in] recv tuner receiver handle
 * @param[out] pids handle to array which should be filled with pids
 *                  it must be at least of size returned by sv_tuner_recv_get_max_pids
 * @param[out] pidsCnt how many entries in array was filled
 * @return @c 0 on success, value < 0 on failure
 */
int sv_tuner_recv_get_pids(struct sv_tuner_recv* recv, int **pids, int *pidsCnt);

/** Try to detect format (PMT)
 * @returns 0 on success
 * @returns blocking SV_ERR_*
 * @returns SV_ERR_RESOURCE_LIMIT when the reader was unlinked from its tuner
 * @returns fatal errors.
 */
int sv_tuner_recv_detect_format(struct sv_tuner_recv* recv);

struct svdataformat* sv_tuner_recv_get_format(const struct sv_tuner_recv* recv);

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_TUNER_RECV_H

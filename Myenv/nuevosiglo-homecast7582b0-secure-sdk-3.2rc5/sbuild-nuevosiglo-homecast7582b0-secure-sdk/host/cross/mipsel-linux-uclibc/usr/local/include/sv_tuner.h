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

#ifndef SV_TUNER_H
#define SV_TUNER_H

#include <stdbool.h>
#include <dataformat/sv_data_format.h>

#include <QBTunerTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


//------------------------
// for application
//------------------------

struct sv_tuner_state;
typedef struct sv_tuner_state sv_tuner_t;

sv_tuner_t* sv_tuner_create(const char* name, int tuner_num, int drv_num);
void sv_tuner_destroy(sv_tuner_t* sv_tuner);

sv_tuner_t* sv_tuner_get(int tuner_num);
int sv_tuner_get_num(const sv_tuner_t* sv_tuner);

/**
 * Returns a string with tuner parameters in human-readable form.
 *
 * Returned string follows format and fields described in documentation of QBTunerParamsToString()
 * with addition of two more fields: `sid` and `tunType`.
 * Resulting text can be parsed by sv_tuner_parse_params().
 * Example:
 *
 * `tunType=cab:sid=2008:standard=dvbc:freq=663000:mod=QAM256:symbol=6875`
 *
 * @param[in] sid            pointer to optional SID to be printed
 * @param[in] tuner_type     pointer to optional tuner type to be printed
 * @param[in] params       handle of @ref QBTunerParams to be printed to string
 * @return                  a pointer to a string with correct URI
 **/
char *sv_tuner_print_params(int* sid, QBTunerType* tuner_type, const struct QBTunerParams* params);
/**
 * Reads tuner parameters from a human-readable string.
 *
 * A mirror function to sv_tuner_print_params().
 *
 * @param[in] str             source string that tuner parameters have to be read from
 * @param[out] sid            target pointer for SID read from URI
 * @param[out] tuner_type     target pointer for tuner type read from URI
 * @param[out] params       target object for read tuner parameters
 * @return                  0 on success, otherwise negative (wrong/unreadable string)
 **/
int sv_tuner_parse_params(const char* str, int* sid, QBTunerType* tuner_type, struct QBTunerParams* params);


//------------------------
// custom operations
//------------------------

/** Find out current mux_id on tuner \a tuner_num.
 *  @returns mux_id
 */
struct QBTunerMuxId sv_tuner_get_curr_mux_id(int tuner_num);

/** Untune from current frequency (this stops all receivers).
 */
int sv_tuner_untune(int tuner_num);
int sv_tuner_tune(int tuner_num, const struct QBTunerParams* freq_params);

/** Find out the format of \a channel on tuner \a tuner_num.
 *  \returns 0  on success
 *  \returns SV_ERR_INCOMPATIBLE  iff another frequency is in use
 *  \returns SV_ERR_NOT_FOUND  iff there is not such channel
 *  \returns SV_ERR_WOULDBLOCK  iff tuning/scanning/detection is not finished yet
 */
int sv_tuner_find_channel_format(int tuner_num, int sid, struct QBTunerMuxId *mux_id, struct svdataformat** format_out);

struct svdataformat* sv_tuner_get_pmt(int tuner_num, int sid);

int sv_tuner_find_pmt_pid(int tuner_num, int sid);

int sv_tuner_get_status(sv_tuner_t* sv_tuner, struct QBTunerStatus* status);

int sv_tuner_get_properties(int tuner_num, struct QBTunerProperties* props);

int sv_tuner_get_type(int tuner_tum, QBTunerType *type);

int sv_tuner_check_params(int tuner_num, const struct QBTunerParams* params);

void sv_tuner_set_dependent(int master, int slave, bool dependent);

QBDemux* sv_tuner_get_demux(int tuner_tum);

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_TUNER_H

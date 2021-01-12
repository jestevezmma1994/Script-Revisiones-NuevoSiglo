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

#ifndef QB_TUNER_H
#define QB_TUNER_H

#include "QBTunerTypes.h"

#include <QBDemuxTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void QBTunerInit(void);


int QBTunerGetConfig(const char* subsystem_name,
                     const struct QBTunerConfig** configs, int* config_cnt,
                     const struct QBTunerConfigSet** sets, int* set_cnt);

void QBTunerPrintConfig(const char* subsystem_name);

const struct QBTunerConfigSet* QBTunerGetConfigSet(const char* subsystem_name, int config_set_idx);
int QBTunerFindConfigSetByName(const char* subsystem_name, const char* name);
int QBTunerFindConfigSetByPrefix(const char* subsystem_name, const char* name_prefix);
int QBTunerFindConfigSet(const char* subsystem_name, const char* name_prefix, QBTunerStandard standard);

const struct QBTunerProperties* QBTunerGetPropertiesById(const char* subsystem_name, int id);


/// @param num  zero-based tuner id (from config)
QBTuner* QBTunerOpen(const char* subsystem_name, int num);

void QBTunerClose(QBTuner* tuner);
int  QBTunerTune(QBTuner* tuner, const struct QBTunerParams *params);
void QBTunerUntune(QBTuner* tuner);
bool QBTunerIsTuned(const QBTuner* tuner);
int  QBTunerGetStatus(const QBTuner* tuner, struct QBTunerStatus* status);
void QBTunerGetProperties(const QBTuner* tuner, struct QBTunerProperties* prop);

int QBTunerCheckTunerParams(const QBTuner* tuner, const struct QBTunerParams* params);
bool QBTunerParamsAreEqual(const struct QBTunerParams *paramsA, const struct QBTunerParams *paramsB);

struct QBTunerParams QBTunerGetEmptyParams(void);
bool QBTunerParamsCompatible(const struct QBTunerParams* mux1, const struct QBTunerParams* mux2);
struct QBTunerMuxId QBTunerGetEmptyMuxId(void);
bool QBTunerMuxIdEqual(const struct QBTunerMuxId* mux1, const struct QBTunerMuxId* mux2);
QBTunerMuxIdObj* QBTunerMuxIdObjCreate(const struct QBTunerMuxId* mux_id);

SvType QBTunerParamsObj_getType(void);
SvType QBTunerMuxIdObj_getType(void);
QBTunerParamsObj* QBTunerParamsObjCreate(const struct QBTunerParams* tunerParams);

/*
 * Below functions are depracated. They should be removed ASAP.
 */
void QBTunerSetDependent(int master, int slave, bool dependent);
bool QBTunerIsDependent(int master, int slave);
void QBTunerSetLnbMaster(QBTuner* tuner, const bool isLnbMaster);

/** Set master tuner
 * Master tuner is only tuner allowed to change LNB settings
 * @param subsystemName name of tuner subsystem ie. "native"
 * @param tunerNumber number of master tuner
 * @return 0 on success, -1 on fail
 */
int QBTunerSetMaster(const char* subsystemName, int tunerNumber);

/** Get master tuner number
 * @param subsystemName name of tuner subsystem ie. "native"
 * @return number of master tuner, -1 on fail
 */
int QBTunerGetMaster(const char* subsystemName);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_TUNER_HAL

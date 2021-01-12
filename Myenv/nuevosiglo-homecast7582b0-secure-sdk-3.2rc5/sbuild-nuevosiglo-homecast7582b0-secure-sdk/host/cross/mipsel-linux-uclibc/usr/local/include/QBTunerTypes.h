/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TUNER_TYPES_H
#define QB_TUNER_TYPES_H

#include <QBPlatformHAL/QBPlatformTypes.h>
#include <SvFoundation/SvObject.h>
#include <QBDemuxTypes.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum QBTunerModulation_e;
typedef enum QBTunerModulation_e  QBTunerModulation;

enum QBTunerPolarization_e;
typedef enum QBTunerPolarization_e  QBTunerPolarization;

enum QBTunerStandard_e;
typedef enum QBTunerStandard_e  QBTunerStandard;

enum QBTunerSatelliteMode_e;
typedef enum QBTunerSatelliteMode_e  QBTunerSatelliteMode;


const char* QBTunerTypeToString(QBTunerType type);
QBTunerType QBTunerTypeFromString(const char* str);
int QBTunerTypeSprintfAll(char* buf, int len);


enum QBTunerModulation_e
{
  QBTunerModulation_unknown = 0,
  QBTunerModulation_default = QBTunerModulation_unknown, // obsolete

  QBTunerModulation_QAM16,
  QBTunerModulation_QAM32,
  QBTunerModulation_QAM64,
  QBTunerModulation_QAM128,
  QBTunerModulation_QAM256,

  QBTunerModulation_QPSK,
  QBTunerModulation_8PSK,

  QBTunerModulation_cnt,
};

enum QBTunerStandard_e
{
  QBTunerStandard_unknown = 0,
  QBTunerStandard_DVBC,
  QBTunerStandard_ANNEXB,

  QBTunerStandard_cnt,
};

enum QBTunerSatelliteMode_e
{
    QBTunerSatelliteMode_unknown = 0,
    QBTunerSatelliteMode_dvbs,
    QBTunerSatelliteMode_dvbs2,

    QBTunerSatelliteMode_cnt,
};

const char* QBTunerStandardToString(QBTunerStandard modulation);
const char* QBTunerSatelliteModeToString(QBTunerSatelliteMode satMode);
const char* QBTunerStandardToPrettyString(QBTunerStandard modulation);
QBTunerStandard QBTunerStandardFromString(const char* str);
QBTunerSatelliteMode QBTunerSatelliteModeFromString(const char* str);

const char* QBTunerModulationToString(QBTunerModulation modulation);
int QBTunerModulationSprintfAll(char* buf, int len);

QBTunerModulation QBTunerModulationFromString(const char* str);
QBTunerModulation QBTunerModulationFromQAM(int qam_mode);
QBTunerModulation QBTunerModulationFromSatelliteModulationType(int modulation_type);


enum QBTunerPolarization_e
{
  QBTunerPolarization_unknown = 0,
  QBTunerPolarization_default = QBTunerPolarization_unknown, // obsolete

  QBTunerPolarization_Vertical,
  QBTunerPolarization_Horizontal,

  QBTunerPolarization_cnt,
};

const char* QBTunerPolarizationToString(QBTunerPolarization polarization);
const char* QBTunerPolarizationToShortString(QBTunerPolarization polarization);
QBTunerPolarization QBTunerPolarizationFromString(const char* str);
QBTunerPolarization QBTunerPolarizationFromDescriptor(uint8_t polarization);
int QBTunerPolarizationSprintfAll(char* buf, int len);


typedef enum {
  QBTunerLNBType_unknown = 0,
  QBTunerLNBType_Ku_universal,
  QBTunerLNBType_Ku_low,
  QBTunerLNBType_Ku_FSS,
  QBTunerLNBType_Ku_BSS,
  QBTunerLNBType_C,

  QBTunerLNBType_cnt,
} QBTunerLNBType;

const char* QBTunerLNBTypeToString(QBTunerLNBType lnb_type);

struct QBTunerLNBParams
{
  QBTunerLNBType type;
  bool dual_band;
  struct {
    unsigned int freq_min, freq_max;
    unsigned int local_osc_freq;
  } bands[2];
  bool enable_22kHz_tone;
  /// DiSEqC switch port number, @c -1 to disable DiSEqC
  int DiSEqC_port_number;

};

int QBTunerGetLNBParams(QBTunerLNBType lnb_type, struct QBTunerLNBParams *params);
bool QBTunerLNBParamsAreEqual(const struct QBTunerLNBParams *paramsA, const struct QBTunerLNBParams *paramsB);


struct QBTunerMuxId
{
    int freq;
    int plp_id;
    uint32_t custom0;
    char satelliteID[16];
};

struct QBTunerParams
{
    struct QBTunerMuxId mux_id;
    QBTunerModulation modulation;
    int symbol_rate;
    int band;
    QBTunerPolarization polarization;
    int extra_val;
    int spectral_inv;
    int freq_offset;
    QBTunerStandard standard;
    QBTunerSatelliteMode satelliteMode;
};

struct QBTunerParamsObj_s {
  struct SvObject_ super_;
  struct QBTunerParams params;
};
typedef struct QBTunerParamsObj_s QBTunerParamsObj;

struct QBTunerStatus
{
  bool fast_lock;
  bool full_lock;
  bool should_have_lock;

  double snr; // dB
  double preber;
  double postber;
  double signal_level; // dB

  double short_term_error; // The average value of post BER per unit of time
  double long_term_error; // The average value of post BER per unit of time

  int signal_strength;
  int signal_quality;

  struct QBTunerParams params;
  struct QBTunerParams detected_params;

  int max_hierarchy;
};

struct QBTunerProperties
{
  const char* sw_version;

  QBTunerType type;

  struct {
    int freq_min;
    int freq_max;

    int band_min;
    int band_max;

    int symbol_rate_min;
    int symbol_rate_max;

    const QBTunerModulation* modulation_tab;
    int modulation_cnt;

    const QBTunerPolarization* polarization_tab;
    int polarization_cnt;

    const QBTunerStandard* standard_tab;
    int standard_cnt;

    const QBTunerSatelliteMode* satelliteMode_tab;
    int satelliteMode_cnt;

    const int *spectral_inv_tab;
    int spectral_inv_tab_cnt;

    int extra_cnt;
  } allowed;

  struct {
    int band;
    int symbol_rate;
    QBTunerModulation modulation;
    QBTunerPolarization polarization;
    QBTunerStandard standard;
    QBTunerSatelliteMode satelliteMode;
    int extra_val;
    int spectral_inv;
    int plp_id;
  } defaults;

  struct {
    bool band;
    bool symbol_rate;
    bool modulation;
    bool polarization;
    bool spectral_inv;
    bool extra_val;
    bool plp_id;
    bool standard;
    bool satelliteMode;
  } automatic;

  struct {
      const char* extra_name;
      const char**const* extra_val_name;
  } names;
};


struct QBTunerConfig
{
  int id; // unique in the subsystem
  const struct QBTunerProperties* props;
  const char* debug_name;
};

struct QBTunerConfigSet
{
  const char* name;
  const char* full_name;
  int cnt;
  const int* tab; /// ids of tuners in this set
};



struct QBTunerFunctor;

struct QBTuner_s
{
  const struct QBTunerFunctor* functor;

  int num;
  QBTunerType type;
  QBTunerStandard standard;
  QBDemux* demux;
} __attribute__ ((aligned (8)));
typedef struct QBTuner_s  QBTuner;

struct QBTunerMuxIdObj_ {
  struct SvObject_ super_;
  struct QBTunerMuxId mux_id;
};
typedef struct QBTunerMuxIdObj_ QBTunerMuxIdObj;

/// Special values for PIDs to be received.
enum {
  QBTunerPidWildcard = 0xffff,
};

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_TUNER_TYPES_H

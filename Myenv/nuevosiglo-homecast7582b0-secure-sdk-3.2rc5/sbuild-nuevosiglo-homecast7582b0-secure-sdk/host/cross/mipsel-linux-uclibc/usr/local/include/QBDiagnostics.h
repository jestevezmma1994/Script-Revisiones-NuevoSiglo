/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBDIAGNOSTIC_H_
#define QBDIAGNOSTIC_H_

#include <SvFoundation/SvCoreTypes.h> // SvInterface
#include <CUIT/Core/types.h> // SvApplication
#include <QBTunerTypes.h> // QBTunerModulation
#include <SvEPGDataLayer/Plugins/DVBEPGChannel.h> // DVBEPGChannel
#include <QBDVBTableMonitor.h>
#include <dataformat/sv_data_format.h>
#include <stdbool.h>

typedef enum {
    QBDiagnosticsChangeType_unknown = 0,
    QBDiagnosticsChangeType_Modulation,
    QBDiagnosticsChangeType_Polarization,
    QBDiagnosticsChangeType_CarrierLock,
    QBDiagnosticsChangeType_RelocksCounter,
    QBDiagnosticsChangeType_SignalLevel,
    QBDiagnosticsChangeType_SignalStrength,
    QBDiagnosticsChangeType_QualityLevel,
    QBDiagnosticsChangeType_PreBER,
    QBDiagnosticsChangeType_SNR,
    QBDiagnosticsChangeType_PATDVBTableChanged,
    QBDiagnosticsChangeType_PMTDVBTableChanged,
    QBDiagnosticsChangeType_DVBTableRepeated,
    QBDiagnosticsChangeType_ShortTermError,
    QBDiagnosticsChangeType_LongTermError,
    QBDiagnosticsChangeType_Bandwidth,
    QBDiagnosticsChangeType_SymbolRate,
    QBDiagnosticsChangeType_cnt,
} QBDiagnosticsChangeType;

typedef struct QBDiagnostics_ *QBDiagnostics;

typedef const struct QBDiagnosticsListener_t {
    void (*stateChanged)(SvObject self_, QBDiagnosticsChangeType type);
} *QBDiagnosticsListener;

extern SvInterface QBDiagnosticsListener_getInterface(void);

struct QBDiagnosticProcStat_s {
    long num_threads;
    unsigned long vsize; // size in bytes
    unsigned long rss; // number of pages
    double uptimeSec;
    double idleSec;
    double userTimeSec;
    double kernelTimeSec;
    /// total memory (/proc/meminfo)
    unsigned long totalMem;
    /// used memory (/proc/meminfo)
    unsigned long usedMem;
    /// free memory (/proc/meminfo)
    unsigned long freeMem;
    /// buffered data in memory (/proc/meminfo)
    unsigned long buffMem;
};
typedef struct QBDiagnosticProcStat_s QBDiagnosticProcStat;


// EIT, TDT is not parse by DVB Table Monitor
typedef struct QBDiagnosticDVBTableStatus_s QBDiagnosticDVBTableStatus;
struct QBDiagnosticDVBTableStatus_s {
    bool PAT;
    bool PMT;
    bool CAT;
    bool NIT;
    bool SDT;
};

typedef struct QBDiagnosticsPATElement_t* QBDiagnosticsPATElement;
struct QBDiagnosticsPATElement_t {
    struct SvObject_ super_;
    int program_number;
    int PID;
};

SvType QBDiagnosticsPATElement_getType(void);

typedef struct QBDiagnosticsPMTVideo_t* QBDiagnosticsPMTVideo;
struct QBDiagnosticsPMTVideo_t {
    struct SvObject_ super_;
    int index; // index of video element in dataformat
    int pid;
    char* codec;
    enum svdataformattype type;
};

SvType QBDiagnosticsPMTVideo_getType(void);

typedef struct QBDiagnosticsPMTAudio_t* QBDiagnosticsPMTAudio;
struct QBDiagnosticsPMTAudio_t {
    struct SvObject_ super_;
    int index; // index of video element in dataformat
    int pid;
    char* lang;
    QBAudioCodec codec;
    QBAudioType type;
    QBAudioDualMonoMode dual_mono_mode;
};

SvType QBDiagnosticsPMTAudio_getType(void);

typedef struct QBDiagnosticsPMTElement_t* QBDiagnosticsPMTElement;
struct QBDiagnosticsPMTElement_t {
    struct SvObject_ super_;
    int version_number;
    int program_number;
    int pcr_pid;
    int sid;
    SvArray video;
    SvArray audio;
};

SvType QBDiagnosticsPMTElement_getType(void);

/**
 * Create QBDiagnostics.
 *
 * @param [in] tvChannel    tv channel
 * @param [in] tunerNumber  tuner number
 * @return                  created QBDiagnostics
 **/
extern QBDiagnostics QBDiagnosticsCreate(SvTVChannel tvChannel, int tunerNumber);

void QBDiagnosticsStart(QBDiagnostics self);
void QBDiagnosticsStop(QBDiagnostics self);

extern void QBDiagnosticsAddListener(QBDiagnostics self, SvObject listener);
extern void QBDiagnosticsRemoveListener(QBDiagnostics self, SvObject listener);

bool QBDiagnosticsIsAudioAvailable(QBDiagnostics self);
bool QBDiagnosticsIsVideoAvailable(QBDiagnostics self);

/**
 * @brief Check if signal is active or not.
 * @ret 0 - false, 1 - true, < 0 - not defined
 */
int QBDiagnosticsGetCarrierLock(QBDiagnostics self);

int QBDiagnosticsGetTSID(QBDiagnostics self);
int QBDiagnosticsGetSID(QBDiagnostics self);
float QBDiagnosticsGetSignalLevel(QBDiagnostics self);
int QBDiagnosticsGetSignalStrength(QBDiagnostics self);
int QBDiagnosticsGetQualityLevel(QBDiagnostics self);
float QBDiagnosticsGetPreBER(QBDiagnostics self);
float QBDiagnosticsGetPreSNR(QBDiagnostics self);
float QBDiagnosticsGetSNR(QBDiagnostics self);
double QBDiagnosticsGetShortTermError(QBDiagnostics self);
double QBDiagnosticsGetLongTermError(QBDiagnostics self);
int QBDiagnosticsGetBandwidth(QBDiagnostics self);
int QBDiagnosticsGetSymbolRate(QBDiagnostics self);
SvArray QBDiagnosticsGetPMT(QBDiagnostics self);
SvArray QBDiagnosticsGetPAT(QBDiagnostics self);
QBTunerModulation QBDiagnosticsGetModulationMode(QBDiagnostics self);
QBTunerPolarization QBDiagnosticsGetPolarizationMode(QBDiagnostics self);
const QBDiagnosticProcStat* QBDiagnosticsGetCpuMemoryStatus(QBDiagnostics self);
const QBDiagnosticDVBTableStatus* QBDiagnosticsGetDVBTableStatus(QBDiagnostics self);

#endif //! QBDIAGNOSTIC_H_

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

#ifndef QBSTANDBYDVBBYSCAN_H_
#define QBSTANDBYDVBBYSCAN_H_

#include <SvCore/SvErrorInfo.h>
#include <QBDVBScanner.h>
#include <tunerReserver.h>
#include <main_decl.h>


typedef struct QBStandbyDVBScan_ *QBStandbyDVBScan;


typedef bool (*QBStandbyDVBScanPrepareParams)(void *self_,
                                              QBStandbyDVBScan scan,
                                              QBDVBScannerParams* params);

typedef void (*QBStandbyDVBScanOnStart)(void *self_,
                                        QBStandbyDVBScan scan,
                                        QBDVBScanner* scanner);

typedef void (*QBStandbyDVBScanOnFinish)(void *self_,
                                         QBStandbyDVBScan scan,
                                         QBDVBScanner* scanner,
                                         bool forced);


struct QBStandbyDVBScanCallbacks_ {
    QBStandbyDVBScanPrepareParams prepareParams;
    QBStandbyDVBScanOnFinish      onFinish;
    QBStandbyDVBScanOnStart       onStart;
};
typedef struct QBStandbyDVBScanCallbacks_ QBStandbyDVBScanCallbacks;



QBStandbyDVBScan
QBStandbyDVBScanCreate(AppGlobals appGlobals,
                       int priority,
                       SvErrorInfo *errorOut);

void
QBStandbyDVBScanSetCallbacks(QBStandbyDVBScan self,
                             QBStandbyDVBScanCallbacks* callbacks,
                             void *callbackData);

int
QBStandbyDVBScanStart(QBStandbyDVBScan self,
                      SvErrorInfo *errorOut);

/**
 * QBStandbyDVBScannerStartWithTunerResvParam starting scanning with scanner and tuner reserver params.
 *
 * @param[in] self instance of dvb scanner
 * @param[in] resvParams params of tuner reservation
 * @param[out] errorOut error output
 * @return     0 on success or -1 when error has occurred (check error output)
 **/
int
QBStandbyDVBScanStartWithTunerResvParam(QBStandbyDVBScan self,
                                        const QBTunerResvParams *resvParams,
                                        SvErrorInfo *errorOut);

void
QBStandbyDVBScanStop(QBStandbyDVBScan self);

int
QBStandbyDVBScanGetTunerNum(QBStandbyDVBScan self);

#endif // QBSTANDBYDVBBYSCAN_H_

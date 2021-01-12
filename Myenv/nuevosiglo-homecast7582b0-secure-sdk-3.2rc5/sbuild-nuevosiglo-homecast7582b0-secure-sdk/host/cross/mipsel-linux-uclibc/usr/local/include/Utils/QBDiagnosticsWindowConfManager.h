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


#ifndef QBDIAGNOSTICWINDOWCONF_H_
#define QBDIAGNOSTICWINDOWCONF_H_

#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>
#include <stdbool.h>

typedef struct QBDiagnosticsManager_t *QBDiagnosticsManager;
typedef struct QBDiagnosticsWindowConf_t *QBDiagnosticsWindowConf;
typedef struct QBDiagnosticInfo_t *QBDiagnosticInfo;

/**
 * Get runtime type identification object representing QBDiagnosticsManager type.
 *
 * @return type identification object
**/
SvType
QBDiagnosticsManager_getType(void);

/**
 * Create QBDiagnosticsManager.
 *
 * @param[in]  fileName     file name containing diagnostics configuration
 * @return                  created diagnostics manager
 **/
QBDiagnosticsManager
QBDiagnosticsManagerCreate(SvString fileName);

QBDiagnosticsWindowConf QBDiagnosticsManagerGetConf(QBDiagnosticsManager self, SvString id);

SvArray QBDiagnosticsWindowConfGetDiagnostics(QBDiagnosticsWindowConf self);
bool QBDiagnosticsWindowConfIsEnabled(QBDiagnosticsWindowConf, SvString diagnosticId);
SvString QBDiagnosticsWindowConfGetCaption(QBDiagnosticsWindowConf self);

SvString QBDiagnosticInfoGetId(QBDiagnosticInfo self);
SvString QBDiagnosticInfoGetLabel(QBDiagnosticInfo self);
/**
 * Get indentation of diagnostic info field.
 *
 * @param[in] self  QBDiagnosticInfo handle
 * @return          indentation of diagnostic info field
 **/
int QBDiagnosticInfoGetIndentation(QBDiagnosticInfo self);

#endif //! QBDIAGNOSTICWINDOWCONF_H_

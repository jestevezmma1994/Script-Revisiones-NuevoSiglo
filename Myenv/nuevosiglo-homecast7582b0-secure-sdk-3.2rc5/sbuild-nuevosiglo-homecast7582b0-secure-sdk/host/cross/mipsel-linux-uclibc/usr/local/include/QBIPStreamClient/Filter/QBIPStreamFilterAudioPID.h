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

#ifndef QBIPSTREAMCLIENT_QBIPSTREAMFILTERAUDIOPID_H_
#define QBIPSTREAMCLIENT_QBIPSTREAMFILTERAUDIOPID_H_

/**
 * @file QBIPStreamFilterAudioPID.h
 * @brief Audio PID extractor filter
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvType.h>

/**
 * @defgroup QBIPStreamFilterAudioPID Audio PID extractor filter
 * @ingroup QBHLSManager
 * @{
 * Audio PID extractor filter
 **/

/**
 * QBIPStreamFilterAudioPID
 * @class QBIPStreamFilterAudioPID
 * @extends SvObject
 */
typedef struct QBIPStreamFilterAudioPID_ *QBIPStreamFilterAudioPID;

/**
 * Get runtime type identification object representing QBIPStreamFilterAudioPID class.
 *
 * @return audio PID filter class
 **/
SvType
QBIPStreamFilterAudioPID_getType(void);

/**
 * Get audio stream PID.
 *
 * @param[in]  self filter handle
 * @param[out] pid  parsed pid
 * @return @c true when PID is already known, @c false when opposite
 */
bool
QBIPStreamFilterAudioPIDGetAudioPid(QBIPStreamFilterAudioPID self, int *pid);

/**
 * Get PAT table.
 *
 * Returned PAT table must not be modified.
 *
 * @param[in] self  filter handle
 * @param[out] pat  parset PAT table, TS packetized
 * @param[out] size size of PAT table in bytes
 * @return @c true when PAT is already known, @c false when opposite
 */
bool
QBIPStreamFilterAudioPIDGetPat(QBIPStreamFilterAudioPID self, const char **pat, size_t *size);

/**
 * Get PMT table.
 *
 * Returned PAT table must not be modified.
 *
 * @param[in] self  filter handle
 * @param[out] pmt  parset PMT table, TS packetized
 * @param[out] size size of PMT table in bytes
 * @return @c true when PAT is already known, @c false when opposite
 */
bool
QBIPStreamFilterAudioPIDGetPmt(QBIPStreamFilterAudioPID self, const char **pmt, size_t *size);

/**
 * Create new filter.
 *
 * Filter is transparent for data, but it extracts audio track PID and releated PAT and PMT tables.
 *
 * @param[out] errorOut error handle
 * @return new filter handle or @c NULL on error
 */
QBIPStreamFilterAudioPID
QBIPStreamFilterAudioPIDCreate(SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBIPSTREAMCLIENT_QBIPSTREAMFILTERAUDIOPID_H_ */

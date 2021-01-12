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

#ifndef QBIPSTREAMCLIENT_QBIPSTREAMFILTERREMOVEAUDIO_H_
#define QBIPSTREAMCLIENT_QBIPSTREAMFILTERREMOVEAUDIO_H_

/**
 * @file QBIPStreamFilterAudioRemove.h
 * @brief Audio remover filter
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <QBIPStreamClient/Filter/QBIPStreamFilterAudioPID.h>

/**
 * @defgroup QBIPStreamFilterAudioRemove Audio remover filter
 * @ingroup QBHLSManager
 * @{
 * Audio remover filter
 **/

/**
 * QBIPStreamFilterAudioRemove
 * @class QBIPStreamFilterAudioRemove
 * @extends SvObject
 */
typedef struct QBIPStreamFilterAudioRemove_ *QBIPStreamFilterAudioRemove;

/**
 * Create new filter.
 *
 * Filter removes TS segments with audio PID.
 *
 * Filter must be connected aftr 188 filter.
 *
 * @param[in]  filterAudioPid filter audio pid handle
 * @param[out] errorOut errorInfo
 * @return     filter handle or @c NULL on error
 */
QBIPStreamFilterAudioRemove
QBIPStreamFilterAudioRemoveCreate(QBIPStreamFilterAudioPID filterAudioPid,
                                  SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBIPSTREAMCLIENT_QBIPSTREAMFILTERREMOVEAUDIO_H_ */

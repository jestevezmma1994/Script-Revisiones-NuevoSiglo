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
#ifndef CHANNEL_LOGOS_H_
#define CHANNEL_LOGOS_H_

#include <Logic/TunerLogic.h>
#include <Services/core/playlistManager.h>
#include <QBDVBTableMonitor.h>
#include <SvFoundation/SvString.h>

/**
 * @file QBDVBChannelLogos.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief Service used to set channel logos over DSM-CC.
 * Service looks up a linkage descriptor and sets logos downloaded using DSM-CC
 **/

/**
 * QBDVBChannelLogosService class.
 *
 * This class implements a service that fetches logos of channels via DSM-CC and sets them in UI.
 **/
typedef struct QBDVBChannelLogosService_ *QBDVBChannelLogosService;

/**
 * Create QBDVBChannelLogosService
 * @param [in] confPath       absolute path to configuration file of service
 * @param [in] tunerLogic     QBTunerLogic handle
 * @param [in] playlists      QBPlaylistManager  handle
 * @return                    the instance of QBDVBChannelLogosService or NULL if error occured
 **/
QBDVBChannelLogosService
QBDVBChannelLogosServiceCreate(SvString confPath, QBTunerLogic tunerLogic, QBPlaylistManager playlists);

/**
 * Start QBDVBChannelLogosService service. After service is started, it listens to events from table monitor and dsmcc client
 * See also QBDVBChannelLogosServiceStop().
 * @param [in] self           QBDVBChannelLogosService handle
 **/
void QBDVBChannelLogosServiceStart(QBDVBChannelLogosService self);

/**
 * Stop QBJSONSerializer service. After service is stopped, it does not listen to any event nor it updates channel logos
 * See also QBDVBChannelLogosServiceStart().
 * @param [in] self           QBDVBChannelLogosService handle
 **/
void QBDVBChannelLogosServiceStop(QBDVBChannelLogosService self);

#endif

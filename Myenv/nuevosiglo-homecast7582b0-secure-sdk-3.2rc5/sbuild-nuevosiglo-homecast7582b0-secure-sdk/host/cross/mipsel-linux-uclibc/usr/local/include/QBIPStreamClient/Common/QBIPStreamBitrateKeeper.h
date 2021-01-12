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

#ifndef QB_IP_STREAM_BITRATE_KEEPER_H_
#define QB_IP_STREAM_BITRATE_KEEPER_H_

/**
 * @file  QBIPStreamBitrateKeeper.h
 * @brief Streaming bitrate keeper. Used to storage real download bitrate in file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBIPStreamBitrateKeeper QBIPStreamBitrateKeeper: Streaming bitrate keeper
 * @{
 **/

/**
 * Streaming bitrate keeper used to storage real download bitrate in file.
 *
 * @class QBIPStreamBitrateKeeper
 * @extends SvObject
 **/
typedef struct QBIPStreamBitrateKeeper_* QBIPStreamBitrateKeeper;

/**
 * Create IP stream bitrate keeper.
 *
 * @memberof QBIPStreamBitrateKeeper
 * @param[out] errorOut     error info
 * @return                  new created IP stream bitrate keeper,
 *                          otherwise @c NULL in case of error
 **/
extern QBIPStreamBitrateKeeper
QBIPStreamBitrateKeeperCreate(SvErrorInfo *errorOut);

/**
 * Get previous bitrate.
 *
 * Get bitrate set by QBIPStreamBitrateKeeperUpdateBitrate() in this
 * QBIPStreamBitrateKeeper instance or in the previous one
 * (previous bitrate is stored in file).
 *
 * @memberof QBIPStreamBitrateKeeper
 *
 * @param[in] self      streaming bitrate keeper handle
 * @return              previous bitrate (bits per second)
 **/
extern int
QBIPStreamBitrateKeeperGetBitrate(QBIPStreamBitrateKeeper self);

/**
 * Update bitrate.
 *
 * @memberof QBIPStreamBitrateKeeper
 *
 * @param[in] self              streaming bitrate keeper handle
 * @param[in] newBitrate        new bitrate (bits per second)
 **/
extern void
QBIPStreamBitrateKeeperUpdateBitrate(QBIPStreamBitrateKeeper self,
                                     int newBitrate);

/**
 * @}
 **/

#endif /* QB_IP_STREAM_BITRATE_KEEPER_H_ */

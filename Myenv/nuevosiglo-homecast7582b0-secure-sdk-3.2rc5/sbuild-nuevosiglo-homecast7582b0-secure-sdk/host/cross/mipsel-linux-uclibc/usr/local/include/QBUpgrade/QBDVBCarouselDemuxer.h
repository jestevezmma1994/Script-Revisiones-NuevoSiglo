/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DVB_CAROUSEL_DEMUXER_H_
#define QB_DVB_CAROUSEL_DEMUXER_H_

/**
 * @file QBDVBCarouselDemuxer.h
 * @brief DVB carousel demultiplexer class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDVBCarouselDemuxer DVB carousel demultiplexer class
 * @ingroup QBUpgrade
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURI.h>
#include <QBUpgrade/QBDVBUpgradeSelector.h>


/**
 * DVB carousel demultiplexer class.
 * @class QBDVBCarouselDemuxer
 * @extends SvObject
 **/
typedef struct QBDVBCarouselDemuxer_ *QBDVBCarouselDemuxer;


/**
 * Get runtime type identification object
 * representing QBDVBCarouselDemuxer class.
 *
 * @return QBDVBCarouselDemuxer type identification object
 **/
extern SvType
QBDVBCarouselDemuxer_getType(void);

/**
 * Initialize DVB carousel demultiplexer.
 *
 * @memberof QBDVBCarouselDemuxer
 *
 * @param[in] self      DVB carousel demultiplexer handle
 * @param[in] uri       DVB carousel URI
 * @param[in] selector  upgrade selector values
 * @param[out] allowAnyFirmwareVersion   optional parameter, filled with information if it is forced upgrade - regardless of the new firmware version number
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBDVBCarouselDemuxer
QBDVBCarouselDemuxerInit(QBDVBCarouselDemuxer self,
                         SvURI uri,
                         const QBDVBUpgradeSelector *const selector,
                         bool *allowAnyFirmwareVersion,
                         SvErrorInfo *errorOut);

/**
 * Get DVB carousel URI.
 *
 * @memberof QBDVBCarouselDemuxer
 *
 * @param[in] self      DVB carousel demultiplexer handle
 * @return              DVB carousel URI
 **/
extern SvURI
QBDVBCarouselDemuxerGetURI(QBDVBCarouselDemuxer self);

/**
 * Setup tuner to be used by DVB carousel demultiplexer.
 *
 * @memberof QBDVBCarouselDemuxer
 *
 * @param[in] self      DVB carousel demultiplexer handle
 * @param[in] tunerNumber tuner to use, @c -1 to free tuners
 * @param[out] errorOut error info
 **/
extern void
QBDVBCarouselDemuxerSetTuner(QBDVBCarouselDemuxer self,
                             int tunerNumber,
                             SvErrorInfo *errorOut);

/**
 * Add DVB carousel packet receiver.
 *
 * @memberof QBDVBCarouselDemuxer
 *
 * @param[in] self      DVB carousel demultiplexer handle
 * @param[in] receiver  DVB carousel receiver handle
 * @param[in] type      private section type to receive
 * @param[in] SID       DVB carousel SID to receive
 * @param[out] errorOut error info
 **/
extern void
QBDVBCarouselDemuxerAddReceiver(QBDVBCarouselDemuxer self,
                                SvObject receiver,
                                unsigned int type,
                                unsigned int SID,
                                SvErrorInfo *errorOut);

/**
 * Remove registered DVB carousel packet receiver.
 *
 * @memberof QBDVBCarouselDemuxer
 *
 * @param[in] self      DVB carousel demultiplexer handle
 * @param[in] receiver  DVB carousel receiver handle
 * @param[out] errorOut error info
 **/
extern void
QBDVBCarouselDemuxerRemoveReceiver(QBDVBCarouselDemuxer self,
                                   SvObject receiver,
                                   SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif

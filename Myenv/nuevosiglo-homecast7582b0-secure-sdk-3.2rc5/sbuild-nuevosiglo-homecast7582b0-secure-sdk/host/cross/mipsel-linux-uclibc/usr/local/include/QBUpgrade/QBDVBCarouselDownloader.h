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

#ifndef QB_DVB_CAROUSEL_DOWNLOADER_H_
#define QB_DVB_CAROUSEL_DOWNLOADER_H_

/**
 * @file QBDVBCarouselDownloader.h
 * @brief DVB carousel upgrade downloader class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDVBCarouselDownloader DVB carousel upgrade downloader class
 * @ingroup QBUpgrade
 * @{
 **/

#include <stdint.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBUpgrade/QBUpgradeObject.h>
#include <QBUpgrade/QBUpgradeService.h>
#include <QBUpgrade/QBDVBCarouselDemuxer.h>


/**
 * DVB carousel upgrade downloader class.
 *
 * @class QBDVBCarouselDownloader
 * @extends SvObject
 **/
typedef struct QBDVBCarouselDownloader_ *QBDVBCarouselDownloader;


/**
 * Get runtime type identification object
 * representing QBDVBCarouselDownloader class.
 *
 * @return QBDVBCarouselDownloader type identification object
 **/
extern SvType
QBDVBCarouselDownloader_getType(void);

/**
 * Initialize DVB carousel upgrade downloader.
 *
 * @memberof QBDVBCarouselDownloader
 *
 * @param[in] self      DVB carousel upgrade downloader handle
 * @param[in] service   upgrade service handle
 * @param[in] obj       upgrade object handle
 * @param[in] demuxer   DVB carousel demultiplexer handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBDVBCarouselDownloader
QBDVBCarouselDownloaderInit(QBDVBCarouselDownloader self,
                            QBUpgradeService service,
                            QBUpgradeObject obj,
                            QBDVBCarouselDemuxer demuxer,
                            SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif

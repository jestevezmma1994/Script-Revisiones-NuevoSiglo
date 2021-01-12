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

#ifndef QB_XHTML_IMG_DOWNLOADER_H
#define QB_XHTML_IMG_DOWNLOADER_H

/**
 * @file QBXHTMLImagesDownloader.h XHTML images downloader
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBXHTMLWidgets/QBXHTMLNodes.h>
#include <QBDataModel3/QBActiveTree.h>
#include <CAGE/Core/SvBitmap.h>

/**
 * @defgroup QBXHTMLImagesDownloader XHTML images downloader class
 * @ingroup GUI
 * @{
 *
 * class for downloading images needed to properly display XHTML widget
 **/

/**
 * QBXHTMLImagesDownloader Class for downloading images.
 * This class will perform images download. All images will be downloaded through Resource Bundle, thus will be cached
 * as all other images in the application. Images are downloaded asynchronously. When all images are ready a callback is
 * called.
 *
 * @class QBXHTMLImagesDownloader
 * @extends SvObject
 **/
typedef struct QBXHTMLImagesDownloader_ *QBXHTMLImagesDownloader;

/**
 * Callback called when all images for a widget have been downloaded.
 *
 * @param [in] self_    object passed to the downloader as callback object.
 **/
typedef void (*QBXHTMLImagesReady)(void *self_);

/**
 * Method for creating new Images Downloader.
 *
 * @param [in] documentTree     tree representing XHTML document for which images should be downloaded
 * @param [in] callback         function to call upon download completion
 * @param [in] callbackObject   object which should be passed back to callback
 * @param [out] errorOut        error output
 * @return                      New image downloader or @c NULL in case of error.
 **/
QBXHTMLImagesDownloader
QBXHTMLImagesDownloaderCreate(QBActiveTree documentTree,
                              QBXHTMLImagesReady callback,
                              void *callbackObject,
                              SvErrorInfo *errorOut);

/**
 * Method for starting images download.
 *
 * @param [in] self              Downloader handle.
 **/
void
QBXHTMLImagesDownloaderDownloadImages(QBXHTMLImagesDownloader self);

/**
 * Method for obtaining bitmap downloaded for particular node
 *
 * @param [in] self             Downloader handle.
 * @param [in] node             Node for which a bitmap is needed
 * @return                      Bitmap for given node or @c NULL if no bitmap available
 **/
SvBitmap
QBXHTMLImagesDownloaderGetBitmapForNode(QBXHTMLImagesDownloader self, QBXHTMLVisitableNode node);

/**
 * @}
 **/


#endif //QB_XHTML_IMG_DOWNLOADER_H

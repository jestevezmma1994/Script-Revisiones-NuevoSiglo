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

#ifndef QBXHTML_SV_WIDGETS_FACTORY_H
#define QBXHTML_SV_WIDGETS_FACTORY_H

#include "QBXHTMLImagesDownloader.h"
#include <CUIT/Core/types.h>
#include <SvCore/SvErrorInfo.h>

/**
 * Factory for creating widgets.
 * The factory implements @c QBXHTMLWidgetFactory and @c QBNodeVisitor so it can be passed as a parameter to @c QBXHTMLAssemble.
 * During visitation of some nodes (img, input, select etc) factory creates widgets and inserts them into the results hashtable
 * which is later returned as a result of visitation.
 *
 * @class QBXHTMLSvWidgetFactory
 * @extends SvObject
 * @implements QBNodeVisitor
 * @implements QBXHTMLWidgetFactory
 **/
typedef struct QBXHTMLSvWidgetFactory_ *QBXHTMLSvWidgetFactory;

/**
 * Create new instance of QBXHTMLSvWidgetFactory.
 *
 * @param[in] app           application handle
 * @param[in] imgDownloader image downloader containing all images for this widget
 * @param[out] errorOut     error output
 * @return                  new instance of QBXHTMLSvWidgetFactory or @c NULL in case of error
 **/
extern QBXHTMLSvWidgetFactory
QBXHTMLSvWidgetFactoryCreate(SvApplication app, QBXHTMLImagesDownloader imgDownloader, SvErrorInfo* errorOut);


#endif //QBXHTML_SV_WIDGETS_FACTORY_H

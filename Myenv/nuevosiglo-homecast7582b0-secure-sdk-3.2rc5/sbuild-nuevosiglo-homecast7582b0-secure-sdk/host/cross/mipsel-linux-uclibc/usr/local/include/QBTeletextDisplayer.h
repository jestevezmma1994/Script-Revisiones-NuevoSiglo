/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
****************************************************************************/
#ifndef QB_TELETEXT_DISPLAYER_H
#define QB_TELETEXT_DISPLAYER_H

/**
 * @file QBTeletextDisplayer.h
 * @brief Teletext displayer class.
 */

#include <QBTeletextPageParser.h>
#include <QBTeletextPageWidget.h>

#define QB_TTEXT_DISPLAYER_DH_1      0x001
#define QB_TTEXT_DISPLAYER_DH_2      0x002
#define QB_TTEXT_DISPLAYER_SUBTITLE  0x004

typedef struct QBTeletextDisplayer_ *QBTeletextDisplayer;

/**
 * @defgroup QBTeletextDisplayer_create Creating the teletext displayer.
 * @ingroup QBTeletextDisplayer
 * @{
 **/


/**
 * Create new QBTeletextDisplayer widget.
 *
 * This is a convenience method that allocates and initializes
 * QBTeletextDisplayer widget. It will display teletext pages.
 *
 * Appearance of this widget is controlled by the Settings Manager.
 * It will use following parameters:
 *
 *  - width, height : integer, required,
 *
 * @param[in] app             CUIT application context
 * @param[in] teletextManger  QBTeletextManager
 * @param[in] widgetName      widget name
 * @return                    QBTeletextDisplayer widget, NULL in case of error
 **/
SvWidget QBTeletextDisplayerNew(SvApplication app, const char *widgetName);

void QBTeletextDisplayerSetPage(SvWidget w, int pageNum);
void QBTeletextDisplayerRefreshPage(SvWidget w);
void QBTeletextDisplayerSetZoom (SvWidget w, QBTeletextZoom mode);
void QBTeletextDisplayerSetActive(SvWidget w, bool active);
void QBTeletextSetTransparent (SvWidget w, bool transparent);
void QBTeletextDisplayerSetReceiver(SvWidget w, QBTeletextReceiver receiver);
void QBTeletextDisplayerSetHints(SvWidget w, const char* hintedLang);

#endif

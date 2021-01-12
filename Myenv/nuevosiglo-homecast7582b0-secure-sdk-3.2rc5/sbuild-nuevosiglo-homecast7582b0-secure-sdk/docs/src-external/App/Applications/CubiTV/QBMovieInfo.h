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

#ifndef __QB_MOVIE_INFO_H__
#define __QB_MOVIE_INFO_H__

#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>

extern SvWidget
QBMovieInfoCreate(SvApplication app, const char *widgetName,
                  QBTextRenderer renderer);

extern void
QBMovieInfoSetObject(SvWidget w, SvGenericObject object);

/**
 * Inform the widget if the data fetch is still in progress and it is possible
 * that some data may not be available yet. If the fetch is in progress the widget
 * will use loading animation to inform the user that some data may be displayed later.
 *
 * @param[in] w         movie info widget handle
 * @param[in] isLoading flag to discriminate between fetch in progress (@c true) and fetch finished (@c false) cases.
 **/
extern void
QBMovieInfoSetObjectLoadingState(SvWidget w, bool isLoading);

#endif /* __QB_MOVIE_INFO_H__ */

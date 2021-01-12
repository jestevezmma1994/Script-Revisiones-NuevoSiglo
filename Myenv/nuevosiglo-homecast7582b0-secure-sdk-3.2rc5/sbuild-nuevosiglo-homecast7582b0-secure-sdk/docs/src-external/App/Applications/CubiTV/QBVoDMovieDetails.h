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

#ifndef QBVODMOVIEDETAILS_H_
#define QBVODMOVIEDETAILS_H_

#include <QBWindowContext.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentAction.h>
#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>
#include <Logic/VoDLogic.h>
#include <main_decl.h>

struct QBVoDMovieDetailsBasicElements_s {
    SvWidget title;
    SvWidget cover;
    SvWidget similarCarousel;
    SvWidget trailerPreview;
    SvWidget movieInfo;
    SvWidget description;

    SvBitmap listStartMarker;

    int similarCarouseInitialFocusColumn;

    bool usesMovieInfo;
    bool usesSimilarCarousel;
    bool usesTrailerPreview;
};
typedef struct QBVoDMovieDetailsBasicElements_s* QBVoDMovieDetailsBasicElements;

typedef struct QBVoDMovieDetails_t* QBVoDMovieDetails;

SvType
QBVoDMovieDetailsContext_getType(void);

QBWindowContext
QBVoDMovieDetailsContextCreate(AppGlobals appGlobals, QBContentCategory category, SvDBRawObject movie, SvGenericObject provider, SvString serviceId, SvString serviceName, SvObject externalTree);

SvGenericObject
QBVoDMovieDetailsContextGetContentSideMenu(QBWindowContext ctx_);

SvGenericObject
QBVoDMovieDetailsContextCreateContentInfo(QBWindowContext ctx_);

SvWidget
QBVoDMovieDetailsCoverCreate(QBVoDMovieDetails self, SvApplication app, const char *widgetName);

void
QBVoDMovieDetailsFillCover(SvWidget w, SvString attrType_, SvString attrName_, SvString attrNameVal_);

/**
 * A function setting up VOD movie details description.
 *
 * @param[in] self      a handle to the VOD movie details
 * @param[in] object    a handle to the object containing description to be set
 **/
void
QBVoDMovieDetailsFillDescription(QBVoDMovieDetails self, SvGenericObject object);

SvWidget
QBVoDMovieDetailsSimilarCarouselCreate(QBVoDMovieDetails self, SvApplication app, SvString widgetName_, QBTextRenderer renderer, SvString title);

/**
 * A function setting up VOD movie details provider data.
 *
 * @param[in] self      a handle to the VOD movie details
 * @param[in] provider  a handle to the data provider
 * @param[in] tree      a handle to the content tree used by a data provider
 * @param[in] action    a handle to the action handled by a data provider
 **/
void
QBVoDMovieDetailsSetProviderData(QBVoDMovieDetails self, SvGenericObject provider, SvGenericObject tree, SvGenericObject action);

QBVoDLogic
QBVoDMovieDetailsGetVoDLogic(QBVoDMovieDetails self);

SvGenericObject
QBVoDMovieDetailsGetExternalProvider(QBVoDMovieDetails self);

void
QBVoDMovieDetailsPlayTrailer(QBVoDMovieDetails self, SvDBRawObject movie, double position);

void
QBVoDMovieDetailsSetTitleString(QBVoDMovieDetails self, SvString titleString);

#endif
